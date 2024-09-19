#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>

// Skip the setreuid/setregid calls if they are not needed, to avoid unnecessary system calls and potential errors
#define DO_SETREUID (defined(SET_UID) || (defined(RESET_UID) && RESET_UID != 0) || defined(SET_REAL_UID))
#define DO_SETREGID (defined(SET_GID) || (defined(RESET_GID) && RESET_GID != 0) || defined(SET_REAL_GID))

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
    #if DO_SETREUID

    // Calculate the new effective UID
    #if defined(RESET_UID) && RESET_UID != 0
    uid_t set_uid = getuid();
    #elif defined(SET_UID)
    uid_t set_uid = resolve_uid(SET_UID);
    #else
    uid_t set_uid = geteuid();
    #endif

    // Calculate the new real UID
    #ifdef SET_REAL_UID
    uid_t set_real_uid = resolve_uid(SET_REAL_UID);
    #else
    uid_t set_real_uid = getuid();
    #endif

    if (setreuid(set_real_uid, set_uid) == -1) {
        perror("setreuid");
        return 1;
    }

    #endif

    #if DO_SETREGID

    // Calculate the new effective GID
    #if defined(RESET_GID) && RESET_GID != 0
    gid_t set_gid = getgid();
    #elif defined(SET_GID)
    gid_t set_gid = resolve_gid(SET_GID);
    #else
    gid_t set_gid = getegid();
    #endif

    // Calculate the new real GID
    #ifdef SET_REAL_GID
    gid_t set_real_gid = resolve_gid(SET_REAL_GID);
    #else
    gid_t set_real_gid = getgid();
    #endif

    // Set the new GIDs
    if (setregid(set_real_gid, set_gid) == -1) {
        perror("setregid");
        return 1;
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
