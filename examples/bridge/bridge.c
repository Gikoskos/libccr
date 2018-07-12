#ifdef _GNU_SOURCE
# undef _GNU_SOURCE
#endif


#ifdef _POSIX_C_SOURCE
# undef _POSIX_C_SOURCE
#endif

#define _POSIX_C_SOURCE 200112L

#define CCR_MACRO_LIB 1
#include <ccr.h>

#include <errno.h>
#include <string.h>

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <assert.h>
#include <time.h>
#include <unistd.h> //sleep
#include <limits.h> //INT_MAX



#define MAKE_STR(x) #x

#define TERM_RED  "\x1B[31m"
#define TERM_YEL  "\x1B[33m"
#define TERM_MAG  "\x1B[35m"
#define TERM_CYAN  "\x1B[36m"
#define TERM_YONR  "\x1b[94;41m"
#define TERM_BONW  "\x1B[30;47m"
#define TERM_RESET "\x1B[0m"


#define __ERRNOFATAL(call, fail_condition, line, caller, file) \
do { \
    errno = 0; \
    call; \
    int __tmp_err_code = errno; \
    if (fail_condition) { \
        char errbuff[256]; \
        if (!strerror_r(__tmp_err_code, errbuff, 256)) { \
            fprintf(stderr, "%sFatal call:\t%s%s\n" \
                            "%sat %s -> %s():%d%s\n" \
                            "%sReason: \"%s\"%s\n", \
                            TERM_YONR, MAKE_STR(call), TERM_RESET, \
                            TERM_YONR, file, caller, line, TERM_RESET, \
                            TERM_YONR, errbuff, TERM_RESET); \
        } else { \
            fprintf(stderr, "%sFatal call:\t%s%s\n" \
                            "%sat %s -> %s():%d%s\n", \
                            TERM_YONR, MAKE_STR(call), TERM_RESET, \
                            TERM_YONR, file, caller, line, TERM_RESET); \
        } \
        exit(EXIT_FAILURE); \
    } \
} while (0)

#define __RETFATAL(call, line, caller, file) \
do { \
    int __tmp_err_code = call; \
    if (__tmp_err_code) { \
        char errbuff[256]; \
        if (!strerror_r(__tmp_err_code, errbuff, 256)) { \
            fprintf(stderr, "%sFatal call:\t%s%s\n" \
                            "%sat %s -> %s():%d%s\n" \
                            "%sReason: \"%s\"%s\n", \
                            TERM_YONR, MAKE_STR(call), TERM_RESET, \
                            TERM_YONR, file, caller, line, TERM_RESET, \
                            TERM_YONR, errbuff, TERM_RESET); \
        } else { \
            fprintf(stderr, "%sFatal call:\t%s%s\n" \
                            "%sat %s -> %s():%d%s\n", \
                            TERM_YONR, MAKE_STR(call), TERM_RESET, \
                            TERM_YONR, file, caller, line, TERM_RESET); \
        } \
        exit(EXIT_FAILURE); \
    } \
} while (0)

#define ERRNOFATAL(call, fail_condition) __ERRNOFATAL(call, fail_condition, __LINE__, __func__, __FILE__)
#define RETFATAL(call) __RETFATAL(call, __LINE__, __func__, __FILE__)


static void reset_global_state(void);
static void flush_up_to_newline(FILE *fd);
static int read_next_int(int *x, FILE *fd);

typedef enum car_color {
    RED_CAR = 0, BLUE_CAR
} car_color_e;

typedef struct car_thread_data {
    car_color_e clr;
    unsigned int id;
} car_thread_s;

typedef struct thrd_sync_state {
    int onbridge[2], N;
} thrd_sync_state_s;

thrd_sync_state_s global;
CCR_DECLARE(bridge_region)


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
                    if (c == '\n') {
                        return EIO;
                    }
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

    if (!x)
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

const char *colorstr(car_color_e c)
{
    switch (c) {
    case RED_CAR:
        return TERM_RED"Red"TERM_RESET;
    case BLUE_CAR:
        return TERM_CYAN"Blue"TERM_RESET;
    default:
        return TERM_YONR"Invalid car_color_e value"TERM_RESET;
    }
}

void reset_global_state(void)
{
    if (global.N > 0) {

        global.onbridge[0] = global.onbridge[1] = 0;

    }
}

void bridge_enter(car_thread_s *car)
{
    car_color_e other_side = car->clr ^ 1, this_side = car->clr;

    printf("%s car #%02u arrived!\n", colorstr(this_side), car->id);

    CCR_EXEC(bridge_region, (global.onbridge[other_side] == 0) && (global.onbridge[this_side] < global.N), {
        global.onbridge[this_side]++;
        printf("%s car #%02u is on the bridge."
               "%sonbridge[%s] = %d, onbridge[%s] = %d\n",
               colorstr(this_side), car->id,
               (this_side == RED_CAR) ? " \t" : "\t",
               colorstr(RED_CAR), global.onbridge[RED_CAR],
               colorstr(BLUE_CAR), global.onbridge[BLUE_CAR]);
    });
}

void bridge_leave(car_thread_s *car)
{
    car_color_e this_side = car->clr;

    CCR_EXEC(bridge_region, 1, {
        global.onbridge[this_side]--;
        printf("%s car #%02u got off the bridge."
               "%sonbridge[%s] = %d, onbridge[%s] = %d\n",
               colorstr(this_side), car->id,
               (this_side == RED_CAR) ? " \t" : "\t",
               colorstr(RED_CAR), global.onbridge[RED_CAR],
               colorstr(BLUE_CAR), global.onbridge[BLUE_CAR]);
    });
}

void *car_thread(void *arg)
{
    bridge_enter((car_thread_s*)arg);
    sleep(2);
    bridge_leave((car_thread_s*)arg);

    return NULL;
}

int main(void)
{
    pthread_t *thrds;
    car_thread_s *dat;
    FILE *fd;
    unsigned int total_created_cars, total_red_cars, total_blue_cars;

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

    printf("Max number of cars allowed on the bridge: ");
    RETFATAL(read_next_int(&global.N, fd));
    printf("%d\n", global.N);

    if (global.N > 0) {
        CCR_INIT(bridge_region);
        while (1) {
            reset_global_state();

            total_created_cars = ((unsigned int)rand())%20;
            total_red_cars = 1;
            total_blue_cars = 1;

            printf("\n\n%sCreating %u car threads!%s\n\n", TERM_YEL, total_created_cars, TERM_RESET);

            ERRNOFATAL(thrds = malloc(sizeof(pthread_t) * total_created_cars), !thrds);
            ERRNOFATAL(dat = malloc(sizeof(car_thread_s) * total_created_cars), !dat);

            for (unsigned int i = 0; i < total_created_cars; i++) {
                dat[i].clr = ((unsigned int)rand())%2;

                switch (dat[i].clr) {
                case RED_CAR:
                    dat[i].id = total_red_cars++;
                    break;
                case BLUE_CAR:
                    dat[i].id = total_blue_cars++;
                    break;
                default:
                    RETFATAL(0);
                }
                RETFATAL(pthread_create(&thrds[i], NULL, car_thread, (void*)&dat[i]));
            }

            for (unsigned int i = 0; i < total_created_cars; i++) {
                pthread_join(thrds[i], NULL);
            }

            free(thrds);
            free(dat);

            if (getchar() == 'q')
                break;
        }
    }

    return 0;
}
