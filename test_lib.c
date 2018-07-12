#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include "ccr.h"

#define N 10

int counter = 3;
int c = 0;

ccr_s *test_ccr;

void critical_section(void *p)
{
    int i = *(int*)p;
    printf("Thread %d has control! Integer = %d. Will increment integer.\n", i, counter);
    counter++;
    printf("Thread %d still has control! Integer = %d. Will sleep for 1 secs.\n", i, counter);
    sleep(2);
    printf("Thread %d stopped sleeping! Integer = %d. Will decrement integer.\n", i, counter);
    counter--;
    printf("Thread %d leaves! Integer = %d.\n", i, counter);
}

int condition(void *p)
{
    (void)p;
    return c;
}

void *test_thrd(void *p)
{
    int err = ccr_exec(test_ccr, condition, NULL, critical_section, p);
    if (err) {
        errno = err;
        perror("ccr_exec failed");
        exit(EXIT_FAILURE);
    }

    free(p);
    return NULL;
}

int main(void)
{
    printf("%d threads will be created.\n"
           "A global integer is initialized with the value %d.\n"
           "Each thread will increment the integer by one, when\n"
           "they enter the CCR, and decrement it by one when they\n"
           "leave from it. For the CCR to function properly, the\n"
           "integer is expected to have the value %d when all\n"
           "threads are done.\n\n" , N, counter, counter);

    pthread_t *thrds = malloc(sizeof(pthread_t) * N);

    int err = ccr_init(&test_ccr);
    if (err) {
        errno = err;
        perror("ccr_init failed");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < N; i++) {
        int *num = malloc(sizeof(int));

        *num = i;

        pthread_create(&thrds[i], NULL, test_thrd, (void*)num);
    }

    c = 1;

    for (int i = 0; i < N; i++) {
        pthread_join(thrds[i], NULL);
    }

    free(thrds);

    ccr_destroy(test_ccr);

    assert(counter == 3);

    return 0;
}
