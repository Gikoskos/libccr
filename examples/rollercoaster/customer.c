#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h> //sleep


static unsigned int counter = 1;

static void *passenger_thread(void *param);


void *new_customer(void)
{
    pthread_t *id;
    int *num = malloc(sizeof(int));

    *num = counter++;

    ERRNOFATAL(id = malloc(sizeof(pthread_t)), !id);
    RETFATAL(pthread_create(id, NULL, passenger_thread, (void*)num));

    return (void*)id;
}

void **new_customers(int total)
{
    pthread_t **ids = NULL;

    if (total > 0) {
        ERRNOFATAL(ids = malloc(sizeof(pthread_t*) * (total + 1)), !ids);

        for (int i = 0; i < total; i++) {
            ids[i] = new_customer();
        }

        ids[total] = NULL;
    }

    return (void**)ids;
}

void join_customer(void *customer)
{
    if (customer) {
        pthread_join(*(pthread_t*)customer, NULL);
        free(customer);
    }
}

void join_customers(void **customers)
{
    if (customers) {

        for (int i = 0; customers[i] != NULL; i++) {
            join_customer(customers[i]);
        }

        free(customers);

    }
}

void *passenger_thread(void *param)
{
    unsigned int id = *(unsigned int*)param;

    free(param);

    CCR_EXEC(rollercoaster, global.avl > 0 && !global.ride_is_closed, {
        printf("%sPassenger\t#%02u got a seat in the next ride.%s\n", TERM_CYAN, id, TERM_RESET);
        global.avl--;
        if (global.avl == 0) {
            global.ride_is_closed = 1;
        }
    });

    /* wait until the ride is finished */
    CCR_EXEC(rollercoaster, global.ride_is_finished, {
        printf("%sPassenger\t#%02u got off the ride.%s\n", TERM_CYAN, id, TERM_RESET);
        global.avl++;
        if (global.avl == global.N) {
            global.ride_is_finished = global.ride_is_closed = 0;
        }
    });


    return NULL;
}
