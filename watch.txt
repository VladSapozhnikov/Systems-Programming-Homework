#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <utmp.h>
#include <pwd.h>

/* Prototypes from utmplib */
int utmp_open(char *filename);
struct utmp *utmp_next();
int utmp_reload();
int utmp_close();

#define UTMP_FILE      "/var/run/utmp"
#define DEFAULT_SLEEP  300

/* Holds each user's watch state */
struct watch_entry {
    char *name;
    int is_logged_in;
};

/* Check if a string is all digits */
int is_number(const char *s) {
    for ( ; *s; s++) {
        if(!isdigit((unsigned char)*s)) return 0;
    }
    return 1;
}

/* Returns 1 if user is found in utmp, 0 otherwise */
int user_is_logged_in(const char *user) {
    struct utmp *utbufp;

    utmp_open(UTMP_FILE);
    while ((utbufp = utmp_next()) != NULL) {
        if (utbufp->ut_type == USER_PROCESS && strcmp(utbufp->ut_user, user) == 0) {
            utmp_close();
            return 1;
        }
    }
    utmp_close();
    return 0;
}

/* Checks if the program's owner is still logged in */
int owner_still_logged_in(const char *owner) {
    return user_is_logged_in(owner);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr,"Usage: %s [interval] user1 [user2 ...]\n", argv[0]);
        return 1;
    }

    /* Determine sleep interval and watch-list start */
    int interval = DEFAULT_SLEEP;
    int start_index = 1;
    if (is_number(argv[1])) {
        interval = atoi(argv[1]);
        if (interval <= 0) interval = DEFAULT_SLEEP;
        start_index = 2;
        if (argc < 3) {
            fprintf(stderr,"Usage: %s [interval] user1 [user2 ...]\n", argv[0]);
            return 1;
        }
    }
    int watch_count = argc - start_index;
    struct watch_entry *watch_list = malloc(sizeof(struct watch_entry) * watch_count);
    if (!watch_list) {
        perror("malloc");
        return 1;
    }

    /* Fill watch list */
    for (int i = 0; i < watch_count; i++) {
        watch_list[i].name = argv[start_index + i];
        watch_list[i].is_logged_in = 0;
    }

    /* Initial check: which users are currently logged in? */
    for (int i = 0; i < watch_count; i++) {
        if (user_is_logged_in(watch_list[i].name)) {
            watch_list[i].is_logged_in = 1;
        }
    }

    /* Print currently logged-in watchers (step [b]) */
    printf("\n");
    int any_logged_in = 0;
    for (int i = 0; i < watch_count; i++) {
        if (watch_list[i].is_logged_in) {
            if (!any_logged_in) {
                printf("%s", watch_list[i].name);
                any_logged_in = 1;
            } else {
                printf(" %s", watch_list[i].name);
            }
        }
    }
    if (any_logged_in) {
        printf(" are currently logged in\n");
    } else {
        printf("No watched users are currently logged in\n");
    }

    /* Get program owner's username to know when to exit (step [g]) */
    char *owner = getlogin();
    if (!owner) {
        struct passwd *pw = getpwuid(getuid());
        if (pw) owner = pw->pw_name;
    }

    /* Main loop: periodically check utmp (steps [c], [d], [e]) */
    while (1) {
        sleep(interval);

        /* Check if the owner of this process is still logged in */
        if (!owner_still_logged_in(owner)) {
            break; 
        }

        /* Build a fresh snapshot of who is logged in among watch list */
        for (int i = 0; i < watch_count; i++) {
            int currently_in = user_is_logged_in(watch_list[i].name);

            /* If they were out before but now in -> "logged in" */
            if (!watch_list[i].is_logged_in && currently_in) {
                printf("%s logged in\n", watch_list[i].name);
            }
            /* If they were in before but now out -> "logged out" */
            else if (watch_list[i].is_logged_in && !currently_in) {
                printf("%s logged out\n", watch_list[i].name);
            }
            watch_list[i].is_logged_in = currently_in;
        }
    }

    free(watch_list);
    return 0;
}
