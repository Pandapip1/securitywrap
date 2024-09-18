#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>

uid_t resolve_uid(const char *user_or_uid) {
    char *endptr;
    errno = 0;
    uid_t uid = strtol(user_or_uid, &endptr, 10);
    if (errno != 0) {
        perror("strtol");
        exit(1);
    }
    if (*endptr == '\0') {
        // It's a numeric UID
        return uid;
    } else {
        // It's a user name
        struct passwd *pw;
        if ((pw = getpwnam(user_or_uid)) == NULL) {
            perror("getpwnam");
            exit(1);
        }
        return pw->pw_uid;
    }
}

gid_t resolve_gid(const char *group_or_gid) {
    char *endptr;
    errno = 0;
    gid_t gid = strtol(group_or_gid, &endptr, 10);
    if (errno != 0) {
        perror("strtol");
        exit(1);
    }
    if (*endptr == '\0') {
        // It's a numeric GID
        return gid;
    } else {
        // It's a group name
        struct group *grp;
        if ((grp = getgrnam(group_or_gid)) == NULL) {
            perror("getgrnam");
            exit(1);
        }
        return grp->gr_gid;
    }
}

int main(int argc, char *argv[]) {
    // If RESET_UID is set, reset the effective UID to the real UID
    #ifdef RESET_UID
    if (RESET_UID) {
        if (seteuid(getuid()) == -1) {
            perror("seteuid");
            return 1;
        }
    }
    #endif

    // If SET_REAL_UID is specified, set the real UID
    #ifdef SET_REAL_UID
    uid_t set_real_uid = resolve_uid(SET_REAL_UID);
    if (setuid(set_real_uid) == -1) {
        perror("setuid");
        return 1;
    }
    #endif

    // If SET_UID is specified, set the effective UID
    #ifdef SET_UID
    uid_t set_uid = resolve_uid(SET_UID);
    if (set_uid != -1) {
        if (seteuid(set_uid) == -1) {
            perror("seteuid");
            return 1;
        }
    }
    #endif

    // If RESET_GID is set, reset the effective GID to the real GID
    #ifdef RESET_GID
    if (RESET_GID) {
        if (setegid(getgid()) == -1) {
            perror("setegid");
            return 1;
        }
    }
    #endif

    // If SET_REAL_GID is specified, set the real GID
    #ifdef SET_REAL_GID
    gid_t set_real_gid = resolve_gid(SET_REAL_GID);
    if (set_real_gid != -1) {
        if (setgid(set_real_gid) == -1) {
            perror("setgid");
            return 1;
        }
    }
    #endif

    // If SET_GID is specified, set the effective GID
    #ifdef SET_GID
    gid_t set_gid = resolve_gid(SET_GID);
    if (set_gid != -1) {
        if (setegid(set_gid) == -1) {
            perror("setegid");
            return 1;
        }
    }
    #endif

    // Copy argv into a new array with an additional NULL element and with argv[0] replaced with WRAP_EXECUTABLE
    char *argv_nullterm[argc + 1];
    argv_nullterm[0] = WRAP_EXECUTABLE;
    for (int i = 1; i < argc; i++) {
        argv_nullterm[i] = argv[i];
    }
    argv_nullterm[argc] = NULL;

    // Execute the specified executable with its arguments
    execv(WRAP_EXECUTABLE, argv_nullterm);
    // If execv returns, it means an error occurred
    perror("execv");
    exit(1);
}
