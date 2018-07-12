#ifndef CRITICAL_CONDITIONAL_REGION_LIB_HEADER__
#define CRITICAL_CONDITIONAL_REGION_LIB_HEADER__

#if !defined (CCR_MACRO_LIB)

# define CCR_MACRO_LIB 0

#endif //CCR_MACRO_LIB


#if CCR_MACRO_LIB == 1

# include <pthread.h>
# include <stdio.h> //fprintf
# include <stdlib.h> //exit
# include <errno.h> //errno macros
# include <string.h> //strerror_r

# if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE
#  define CCR_HAVE_STRERROR_R
# endif

# ifdef CCR_USE_TERM_COLORS
#  define CCR_TERM_COLOR_RED "\x1b[94;41m"
#  define CCR_TERM_COLOR_RESET "\x1B[0m"
# else
#  define CCR_TERM_COLOR_RED ""
#  define CCR_TERM_COLOR_RESET ""
# endif


# define CCR_MAKE_STR__(x) #x
# define CCR_MAKE_STR(x) CCR_MAKE_STR__(x)

# define CCR_FATAL_STRERROR_R__(call, line, caller, file) \
do { \
    int __tmp_err_code = call; \
    if (__tmp_err_code) { \
        char errbuff[256]; \
        if (!strerror_r(__tmp_err_code, errbuff, 256)) { \
            fprintf(stderr, "%sFatal call:\t%s%s\n" \
                            "%sat %s -> %s():%d%s\n" \
                            "%sReason: \"%s\"%s\n", \
                            CCR_TERM_COLOR_RED, CCR_MAKE_STR(call), CCR_TERM_COLOR_RESET, \
                            CCR_TERM_COLOR_RED, file, caller, line, CCR_TERM_COLOR_RESET, \
                            CCR_TERM_COLOR_RED, errbuff, CCR_TERM_COLOR_RESET); \
        } else { \
            fprintf(stderr, "%sFatal call:\t%s%s\n" \
                            "%sat %s -> %s():%d%s\n", \
                            CCR_TERM_COLOR_RED, CCR_MAKE_STR(call), CCR_TERM_COLOR_RESET, \
                            CCR_TERM_COLOR_RED, file, caller, line, CCR_TERM_COLOR_RESET); \
        } \
        exit(EXIT_FAILURE); \
    } \
} while (0)

# define CCR_FATAL__(call, line, caller, file) \
do { \
    int __tmp_err_code = call; \
    if (__tmp_err_code) { \
        fprintf(stderr, "%sFatal call:\t%s%s\n" \
                        "%sat %s -> %s():%d%s\n" \
                        "%sError code: %d%s\n", \
                        CCR_TERM_COLOR_RED, CCR_MAKE_STR(call), CCR_TERM_COLOR_RESET, \
                        CCR_TERM_COLOR_RED, file, caller, line, CCR_TERM_COLOR_RESET, \
                        CCR_TERM_COLOR_RED, __tmp_err_code, CCR_TERM_COLOR_RESET); \
        exit(EXIT_FAILURE); \
    } \
} while (0)

# ifdef CCR_HAVE_STRERROR_R
#  define CCR_FATAL(call) CCR_FATAL_STRERROR_R__(call, __LINE__, __func__, __FILE__)
# else
#  define CCR_FATAL(call) CCR_FATAL__(call, __LINE__, __func__, __FILE__)
# endif


# define CCR_CONCAT__(x, y) x ## y
# define CCR_CONCAT(x, y) CCR_CONCAT__(x, y)
# define CCR_VARNAME(x, y) CCR_CONCAT(x, CCR_CONCAT(_, y))


# define CCR_DECLARE(label) \
pthread_mutex_t CCR_VARNAME(label, bsem_mtx), \
                CCR_VARNAME(label, q1_mtx), \
                CCR_VARNAME(label, q2_mtx); \
pthread_cond_t CCR_VARNAME(label, bsem_cond), \
               CCR_VARNAME(label, q1_cond), \
               CCR_VARNAME(label, q2_cond); \
int CCR_VARNAME(label, bsem_count), \
    CCR_VARNAME(label, q1_count), \
    CCR_VARNAME(label, q2_count), \
    CCR_VARNAME(label, n1), \
    CCR_VARNAME(label, n2);


# define CCR_INIT(label) \
do { \
    pthread_mutexattr_t attr; \
\
    CCR_FATAL(pthread_mutexattr_init(&attr)); \
    CCR_FATAL(pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK)); \
    CCR_FATAL(pthread_mutex_init(&CCR_VARNAME(label, bsem_mtx), &attr)); \
    CCR_FATAL(pthread_mutex_init(&CCR_VARNAME(label, q1_mtx), &attr)); \
    CCR_FATAL(pthread_mutex_init(&CCR_VARNAME(label, q2_mtx), &attr)); \
    pthread_mutexattr_destroy(&attr); \
\
    CCR_FATAL(pthread_cond_init(&CCR_VARNAME(label, bsem_cond), NULL)); \
    CCR_FATAL(pthread_cond_init(&CCR_VARNAME(label, q1_cond), NULL)); \
    CCR_FATAL(pthread_cond_init(&CCR_VARNAME(label, q2_cond), NULL)); \
\
    CCR_VARNAME(label, bsem_count) = 1; \
    CCR_VARNAME(label, n1) = 0; \
    CCR_VARNAME(label, n2) = 0; \
    CCR_VARNAME(label, q1_count) = 0; \
    CCR_VARNAME(label, q2_count) = 0; \
} while (0)

# define CCR_EXEC(label, cond, body) \
do { \
    CCR_FATAL(pthread_mutex_lock(&CCR_VARNAME(label, bsem_mtx))); \
    while (!CCR_VARNAME(label, bsem_count)) { \
        CCR_FATAL(pthread_cond_wait(&CCR_VARNAME(label, bsem_cond), &CCR_VARNAME(label, bsem_mtx))); \
    } \
    CCR_VARNAME(label, bsem_count) = 0; \
    CCR_FATAL(pthread_mutex_unlock(&CCR_VARNAME(label, bsem_mtx))); \
\
    while (!(cond)) { \
\
        CCR_VARNAME(label, n1)++; \
\
        if (CCR_VARNAME(label, n2) > 0) { \
            CCR_VARNAME(label, n2)--; \
            CCR_FATAL(pthread_mutex_lock(&CCR_VARNAME(label, q2_mtx))); \
            CCR_VARNAME(label, q2_count) = 1; \
            CCR_FATAL(pthread_cond_signal(&CCR_VARNAME(label, q2_cond))); \
            CCR_FATAL(pthread_mutex_unlock(&CCR_VARNAME(label, q2_mtx))); \
        } else { \
            CCR_FATAL(pthread_mutex_lock(&CCR_VARNAME(label, bsem_mtx))); \
            CCR_VARNAME(label, bsem_count) = 1; \
            CCR_FATAL(pthread_cond_signal(&CCR_VARNAME(label, bsem_cond))); \
            CCR_FATAL(pthread_mutex_unlock(&CCR_VARNAME(label, bsem_mtx))); \
        } \
\
        CCR_FATAL(pthread_mutex_lock(&CCR_VARNAME(label, q1_mtx))); \
        while (!CCR_VARNAME(label, q1_count)) { \
            CCR_FATAL(pthread_cond_wait(&CCR_VARNAME(label, q1_cond), &CCR_VARNAME(label, q1_mtx))); \
        } \
        CCR_VARNAME(label, q1_count) = 0; \
        CCR_FATAL(pthread_mutex_unlock(&CCR_VARNAME(label, q1_mtx))); \
\
        CCR_VARNAME(label, n2)++; \
\
        if (CCR_VARNAME(label, n1) > 0) { \
            CCR_VARNAME(label, n1)--; \
            CCR_FATAL(pthread_mutex_lock(&CCR_VARNAME(label, q1_mtx))); \
            CCR_VARNAME(label, q1_count) = 1; \
            CCR_FATAL(pthread_cond_signal(&CCR_VARNAME(label, q1_cond))); \
            CCR_FATAL(pthread_mutex_unlock(&CCR_VARNAME(label, q1_mtx))); \
        } else { \
            CCR_VARNAME(label, n2)--; \
            CCR_FATAL(pthread_mutex_lock(&CCR_VARNAME(label, q2_mtx))); \
            CCR_VARNAME(label, q2_count) = 1; \
            CCR_FATAL(pthread_cond_signal(&CCR_VARNAME(label, q2_cond))); \
            CCR_FATAL(pthread_mutex_unlock(&CCR_VARNAME(label, q2_mtx))); \
        } \
\
        CCR_FATAL(pthread_mutex_lock(&CCR_VARNAME(label, q2_mtx))); \
        while (!CCR_VARNAME(label, q2_count)) { \
            CCR_FATAL(pthread_cond_wait(&CCR_VARNAME(label, q2_cond), &CCR_VARNAME(label, q2_mtx))); \
        } \
        CCR_VARNAME(label, q2_count) = 0; \
        CCR_FATAL(pthread_mutex_unlock(&CCR_VARNAME(label, q2_mtx))); \
    } \
\
    body; \
\
    if (CCR_VARNAME(label, n1) > 0) { \
        CCR_VARNAME(label, n1)--; \
        CCR_FATAL(pthread_mutex_lock(&CCR_VARNAME(label, q1_mtx))); \
        CCR_VARNAME(label, q1_count) = 1; \
        CCR_FATAL(pthread_cond_signal(&CCR_VARNAME(label, q1_cond))); \
        CCR_FATAL(pthread_mutex_unlock(&CCR_VARNAME(label, q1_mtx))); \
    } else if (CCR_VARNAME(label, n2) > 0) { \
        CCR_VARNAME(label, n2)--; \
        CCR_FATAL(pthread_mutex_lock(&CCR_VARNAME(label, q2_mtx))); \
        CCR_VARNAME(label, q2_count) = 1; \
        CCR_FATAL(pthread_cond_signal(&CCR_VARNAME(label, q2_cond))); \
        CCR_FATAL(pthread_mutex_unlock(&CCR_VARNAME(label, q2_mtx))); \
    } else { \
        CCR_FATAL(pthread_mutex_lock(&CCR_VARNAME(label, bsem_mtx))); \
        CCR_VARNAME(label, bsem_count) = 1; \
        CCR_FATAL(pthread_cond_signal(&CCR_VARNAME(label, bsem_cond))); \
        CCR_FATAL(pthread_mutex_unlock(&CCR_VARNAME(label, bsem_mtx))); \
    } \
} while (0)

#else

typedef struct ccr_s ccr_s;
typedef int (*condition_func)(void *param);
typedef void (*cs_body_func)(void *param);

int ccr_init(ccr_s **ccr);
int ccr_exec(ccr_s *ccr, condition_func cond, void *cond_param, cs_body_func body, void *body_param);
void ccr_destroy(ccr_s *ccr);

#endif //CCR_MACRO_LIB

#endif //CRITICAL_CONDITIONAL_REGION_LIB_HEADER__
