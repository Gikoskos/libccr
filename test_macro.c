#define CCR_MACRO_LIB 1
#include "ccr.h"
#include <unistd.h>

#define N 10

int counter = 3;
int c = 0;

CCR_DECLARE(test)

void *test_thrd(void *p)
{
    int i = *(int*)p;
    free(p);
    CCR_EXEC(test, c, {
        printf("Thread %d has control! Counter = %d. Will increment counter.\n", i, counter);
        counter++;
        printf("Thread %d still has control! Counter = %d. Will sleep for 2 secs.\n", i, counter);
        sleep(2);
        printf("Thread %d stopped sleeping! Counter = %d. Will decrement counter.\n", i, counter);
        counter--;
        printf("Thread %d leaves! Counter = %d.\n", i, counter);
    });

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
           "threads are done.\n"
           "Press [ENTER] to start the simulation.", N, counter, counter);

    pthread_t *thrds = malloc(sizeof(pthread_t) * N);

    CCR_INIT(test);

    for (int i = 0; i < N; i++) {
        int *num = malloc(sizeof(int));

        *num = i;

        pthread_create(&thrds[i], NULL, test_thrd, (void*)num);
    }

    CCR_EXEC(test, 1, {
        c = getchar();
    });

    for (int i = 0; i < N; i++) {
        pthread_join(thrds[i], NULL);
    }

    free(thrds);

    assert(counter == 3);

    return 0;
}
