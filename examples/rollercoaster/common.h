#ifndef COMMON_HEADER_ROLLERCOASTER_
#define COMMON_HEADER_ROLLERCOASTER_

/* get XSI-compliant strerror_r */
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


/*********************
 *  Data types to store the global state
 *****************************************/
typedef struct thrd_sync_state {
    int avl, N, ride_is_finished, ride_is_closed;
} thrd_sync_state_s;

CCR_DECLARE(rollercoaster)
thrd_sync_state_s global;


void *new_customer(void);
void **new_customers(int total);
void join_customer(void *customer);
void join_customers(void **customers);

void *init_rollercoaster(void);
void join_rollercoaster(void *rc);


#endif //COMMON_HEADER_ROLLERCOASTER_
