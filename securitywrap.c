#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>

void print_usage(const char *program_name) {
    fprintf(stderr, "Usage: %s [--set-uid <user_or_uid> | --reset-uid ] [--set-real-uid <user_or_uid>] [--set-gid <group_or_gid> | --reset-gid] [--set-real-gid <group_or_gid>] <executable> [args...]\n", program_name);
}

uid_t resolve_uid(const char *user_or_uid) {
    char *endptr;
    uid_t uid = strtol(user_or_uid, &endptr, 10);
    if (*endptr == '\0') {
        // It's a numeric UID
        return uid;
    } else {
        // It's a user name
        struct passwd *pw;
        if ((pw = getpwnam(user_or_uid)) == NULL) {
            fprintf(stderr, "Error: User '%s' not found\n", user_or_uid);
            exit(1);
        }
        return pw->pw_uid;
    }
}

gid_t resolve_gid(const char *group_or_gid) {
    char *endptr;
    gid_t gid = strtol(group_or_gid, &endptr, 10);
    if (*endptr == '\0') {
        // It's a numeric GID
        return gid;
    } else {
        // It's a group name
        struct group *grp;
        if ((grp = getgrnam(group_or_gid)) == NULL) {
            fprintf(stderr, "Error: Group '%s' not found\n", group_or_gid);
            exit(1);
        }
        return grp->gr_gid;
    }
}

int main(int argc, char *argv[]) {
    // If we aren't setuid, we can't change UIDs or GIDs. Throw an error.
    if (getuid() != 0) {
        fprintf(stderr, "Error: wrapper must be setuid root\n");
        return 1;
    }

    // If argv[0] is setuid or setgid root or not owned by root, this is a security vulnerability. Throw an error.
    struct stat st;
    if (argv[0][0] != '/') {
        fprintf(stderr, "Error: wrapper must be an absolute path\n");
        return 1;
    }
    if (stat(argv[0], &st) == -1) {
        perror("stat");
        return 1;
    }
    if ((st.st_mode & S_ISUID) || (st.st_mode & S_ISGID)) {
        fprintf(stderr, "Error: wrapper must not be setuid or setgid\n");
        return 1;
    }
    if (st.st_uid != 0) {
        fprintf(stderr, "Error: wrapper must be owned by root\n");
        return 1;
    }
    if (st.st_gid != 0) {
        fprintf(stderr, "Error: wrapper must be in the root group\n");
        return 1;
    }

    // Options for setting UIDs and GIDs
    uid_t set_uid = -1;
    uid_t set_real_uid = -1;
    int reset_uid = 0;

    gid_t set_gid = -1;
    gid_t set_real_gid = -1;
    int reset_gid = 0;

    // Parse options
    int optind = 1;
    while (true) {
        if (optind >= argc) {
            fprintf(stderr, "Error: Missing executable\n");
            print_usage(argv[0]);
            return 1;
        }
        char *optarg = argv[optind];
        if (strcmp(optarg, "-h") == 0 || strcmp(optarg, "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(optarg, "--set-uid") == 0) {
            optind++;
            if (optind >= argc) {
                fprintf(stderr, "Error: Missing argument for --set-uid\n");
                return 1;
            }
            char *user_or_uid = argv[optind];
            set_uid = resolve_uid(user_or_uid);
        } else if (strcmp(optarg, "--set-real-uid") == 0) {
            optind++;
            if (optind >= argc) {
                fprintf(stderr, "Error: Missing argument for --set-real-uid\n");
                return 1;
            }
            char *user_or_uid = argv[optind];
            set_real_uid = resolve_uid(user_or_uid);
        } else if (strcmp(optarg, "--reset-uid") == 0) {
            reset_uid = 1;
        } else if (strcmp(optarg, "--set-gid") == 0) {
            optind++;
            if (optind >= argc) {
                fprintf(stderr, "Error: Missing argument for --set-gid\n");
                return 1;
            }
            char *group_or_gid = argv[optind];
            set_gid = resolve_gid(group_or_gid);
        } else if (strcmp(optarg, "--set-real-gid") == 0) {
            optind++;
            if (optind >= argc) {
                fprintf(stderr, "Error: Missing argument for --set-real-gid\n");
                return 1;
            }
            char *group_or_gid = argv[optind];
            set_real_gid = resolve_gid(group_or_gid);
        } else if (strcmp(optarg, "--reset-gid") == 0) {
            reset_gid = 1;
        } else {
            break;
        }
        optind++;
    }

    // If set UID and reset UID are both specified, throw an error (same for GID)
    if (reset_uid && (set_uid != -1 || set_real_uid != -1)) {
        fprintf(stderr, "Error: --set-uid and --reset-uid are mutually exclusive\n");
        return 1;
    }
    if (reset_gid && (set_gid != -1 || set_real_gid != -1)) {
        fprintf(stderr, "Error: --set-gid and --reset-gid are mutually exclusive\n");
        return 1;
    }

    // If set real UID and reset UID are both specified, throw an error (same for GID)
    if (reset_uid && set_real_uid != -1) {
        fprintf(stderr, "Error: --set-real-uid and --reset-uid are mutually exclusive\n");
        return 1;
    }
    if (reset_gid && set_real_gid != -1) {
        fprintf(stderr, "Error: --set-real-gid and --reset-gid are mutually exclusive\n");
        return 1;
    }

    // If set real UID is set but set UID is not, set UID to the real UID (same for GID)
    if (set_real_uid != -1 && set_uid == -1) {
        set_uid = set_real_uid;
    }
    if (set_real_gid != -1 && set_gid == -1) {
        set_gid = set_real_gid;
    }

    // The first argument is the executable to run
    char *executable = argv[optind];
    char **exec_args = &argv[optind];

    // Ensure that the executable is an absolute path
    if (executable[0] != '/') {
        fprintf(stderr, "Error: Executable '%s' is not an absolute path\n", executable);
        return 1;
    }

    // Ensure that the executable is owned by root
    if (stat(executable, &st) == -1) {
        perror("stat");
        return 1;
    }
    if (st.st_uid != 0) {
        fprintf(stderr, "Error: Executable '%s' is not owned by root\n", executable);
        return 1;
    }
    if (st.st_gid != 0) {
        fprintf(stderr, "Error: Executable '%s' is not in the root group\n", executable);
        return 1;
    }

    // If the executable is setuid or setgid, the --reset-uid, --reset-gid, --set-uid, and --set-gid options will not work properly
    if ((st.st_mode & S_ISUID) || (st.st_mode & S_ISGID)) {
        fprintf(stderr, "Warning: Executable '%s' is setuid or setgid. securitywrap will not work properly.\n", executable);
    }

    // If --reset-uid is specified, reset the effective UID to the real UID
    if (reset_uid) {
        if (seteuid(getuid()) == -1) {
            perror("seteuid");
            return 1;
        }
    }

    // If --set-real-uid <user_or_uid> is specified, set the real UID
    if (set_real_uid != -1) {
        if (setuid(set_real_uid) == -1) {
            perror("setuid");
            return 1;
        }
    }

    // If --set-uid <user_or_uid> is specified, set the effective UID
    if (set_uid != -1) {
        if (seteuid(set_uid) == -1) {
            perror("seteuid");
            return 1;
        }
    }

    // If --reset-gid is specified, reset the effective GID to the real GID
    if (reset_gid) {
        if (setegid(getgid()) == -1) {
            perror("setegid");
            return 1;
        }
    }

    // If --set-real-gid <group_or_gid> is specified, set the real GID
    if (set_real_gid != -1) {
        if (setgid(set_real_gid) == -1) {
            perror("setgid");
            return 1;
        }
    }

    // If --set-gid <group_or_gid> is specified, set the effective GID
    if (set_gid != -1) {
        if (setegid(set_gid) == -1) {
            perror("setegid");
            return 1;
        }
    }

    // Execute the specified executable with its arguments
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return 1;
    } else if (pid == 0) {
        // Child process
        execvp(executable, exec_args);
        // If execvp returns, it means an error occurred
        perror("execvp");
        exit(1);
    } else {
        // Parent process
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            return 1;
        }
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            fprintf(stderr, "Child process did not exit normally\n");
            return 1;
        }
    }
}
