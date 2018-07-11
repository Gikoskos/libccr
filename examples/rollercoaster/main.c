#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <time.h>
#include <unistd.h> //sleep
#include <limits.h> //INT_MAX



static void init_global_state(void);
static void flush_up_to_newline(FILE *fd);
static int read_next_int(int *x, FILE *fd);


void init_global_state(void)
{
    if (global.N > 0) {

        global.ride_is_finished = 0;
        global.ride_is_closed = 0;
        global.avl = global.N;

    }
}

void flush_up_to_newline(FILE *fd)
{
    int c;

    do {
        c = fgetc(fd);

    } while (c != '\n' && c != EOF);
}

int read_next_int(int *x, FILE *fd)
{
#define INPUT_BUFF_LEN 16
    int c;
    size_t i = 0;
    char input_buff[INPUT_BUFF_LEN];

    while (1) {
        c = fgetc(fd);

        if (c == EOF) {
            break;
        }

        if (isspace(c)) {
            if (!i) {
                do {
                    if (c == '\n')
                        return EIO;
                    c = fgetc(fd);
                } while (isspace(c));
            } else {
                break;
            }
        }

        if (!isdigit(c)) {
            flush_up_to_newline(fd);
            return EIO;
        }

        if (i >= INPUT_BUFF_LEN - 1) {
            flush_up_to_newline(fd);
            return EMSGSIZE;
        }

        input_buff[i++] = (char)c;

    }

    if (!i || !x)
        return EINVAL;

    input_buff[i] = '\0';

    errno = 0;
    long tmp = strtol(input_buff, NULL, 10);
    if (errno) {
        return errno;
    }

    if (tmp > INT_MAX || tmp < INT_MIN) {
        return ERANGE;
    }

    *x = (int)tmp;

    return 0;
#undef INPUT_BUFF_LEN
}

int main(void)
{
    int total_customers;
    FILE *fd;

    setvbuf(stdout, NULL, _IONBF, 0);
    srand(time(NULL));

    printf("Read input from file or stdin? (f/s): ");
    int c = fgetc(stdin);

    if (c == 'f' && fgetc(stdin) == '\n') {
        ERRNOFATAL(fd = fopen("input.txt", "r"), !fd);
    } else if (c == 's' && fgetc(stdin) == '\n') {
        fd = stdin;
    } else {
        printf("Bad input!\n");
        exit(EXIT_FAILURE);
    }

    printf("Max number of passengers allowed on the rollercoaster: ");
    RETFATAL(read_next_int(&global.N, fd));
    printf("%d\n", global.N);


    if (global.N > 0) {

        CCR_INIT(rollercoaster);
        init_global_state();

        init_rollercoaster();

        while (1) {

            total_customers = ((unsigned int)rand())%20;
            printf("%sNumber of customer threads that will be created: %d%s\n\n", TERM_RED, total_customers, TERM_RESET);

            new_customers(total_customers);

            if (getchar() == 'q')
                break;
        }

    }

    return 0;
}
