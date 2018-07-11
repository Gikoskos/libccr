#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h> //sleep


static int initialized;

static void *rollercoaster_thread(void *param);


void *init_rollercoaster(void)
{
    pthread_t *id = NULL;

    if (!initialized) {

        ERRNOFATAL(id = malloc(sizeof(pthread_t)), !id);
        RETFATAL(pthread_create(id, NULL, rollercoaster_thread, NULL));

        initialized = 1;
    }

    return id;
}

void join_rollercoaster(void *rc)
{
    if (rc) {
        pthread_join(*(pthread_t*)rc, NULL);
        free(rc);
    }
}

void *rollercoaster_thread(void *param)
{
    (void)param;

    while (1) {
        printf("\n%s====================================%s"
                "\n%s===Next rollercoaster ride begins===%s"
                "\n%s====================================%s\n",
                TERM_RED, TERM_RESET,
                TERM_RED, TERM_RESET,
                TERM_RED, TERM_RESET);

        printf("Rollercoaster waits until the train is full.\n");
        CCR_EXEC(rollercoaster, (global.avl == 0) && global.ride_is_closed, {});

        /****** riding the coaster ******/
        printf("%sRide begins!%s\n", TERM_BONW, TERM_RESET);
        sleep(2);
        printf("%sRide is finished!%s\n", TERM_BONW, TERM_RESET);
        /********************************/

        CCR_EXEC(rollercoaster, 1, {
            global.ride_is_finished = 1;
        });
    }

    return NULL;
}
