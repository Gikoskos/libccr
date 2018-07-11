#include <ccr.h>

#define _XOPEN_SOURCE 700

#include <pthread.h>
#include <stdlib.h>
#include <errno.h> //errno macros

#define BSEM_WAIT(_bsem) \
do { \
    err = pthread_mutex_lock(&_bsem.mtx); \
    if (err) goto RET; \
    while (!_bsem.count) { \
        err = pthread_cond_wait(&_bsem.cond, &_bsem.mtx); \
        if (err) goto RET; \
    } \
    _bsem.count = 0; \
    err = pthread_mutex_unlock(&_bsem.mtx); \
    if (err) goto RET; \
} while (0)

#define BSEM_POST(_bsem) \
do { \
    err = pthread_mutex_lock(&_bsem.mtx); \
    if (err) goto RET; \
    _bsem.count = 1; \
    err = pthread_cond_signal(&_bsem.cond); \
    if (err) goto RET; \
    err = pthread_mutex_unlock(&_bsem.mtx); \
    if (err) goto RET; \
} while (0)

typedef struct internal_bsem_s {
    pthread_mutex_t mtx;
    pthread_cond_t cond;
    int count;
} internal_bsem_s;

struct ccr_s {
    internal_bsem_s mutex, q1, q2;
    int n1, n2;
};


int ccr_init(ccr_s **ccr)
{
    if (ccr) {
        int err = 0;
        pthread_mutexattr_t attr;

        *ccr = malloc(sizeof(ccr_s));
        if (!*ccr) {
            return ENOMEM;
        }

        err = pthread_mutexattr_init(&attr);
        if (err) goto INIT_FAIL_1;

        err = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
        if (err) goto INIT_FAIL_2;

        err = pthread_mutex_init(&(*ccr)->mutex.mtx, &attr);
        if (err) goto INIT_FAIL_2;

        err = pthread_mutex_init(&(*ccr)->q1.mtx, &attr);
        if (err) goto INIT_FAIL_3;

        err = pthread_mutex_init(&(*ccr)->q2.mtx, &attr);
        if (err) goto INIT_FAIL_4;

        err = pthread_cond_init(&(*ccr)->mutex.cond, NULL);
        if (err) goto INIT_FAIL_5;

        err = pthread_cond_init(&(*ccr)->q1.cond, NULL);
        if (err) goto INIT_FAIL_6;

        err = pthread_cond_init(&(*ccr)->q2.cond, NULL);
        if (err) goto INIT_FAIL_7;

        (*ccr)->mutex.count = 1;
        (*ccr)->n1 = (*ccr)->n2 = 0;
        (*ccr)->q1.count = (*ccr)->q2.count = 0;

        pthread_mutexattr_destroy(&attr);
        goto INIT_SUCCESS;

INIT_FAIL_7:
        pthread_cond_destroy(&(*ccr)->q1.cond);
INIT_FAIL_6:
        pthread_cond_destroy(&(*ccr)->mutex.cond);
INIT_FAIL_5:
        pthread_mutex_destroy(&(*ccr)->q2.mtx);
INIT_FAIL_4:
        pthread_mutex_destroy(&(*ccr)->q1.mtx);
INIT_FAIL_3:
        pthread_mutex_destroy(&(*ccr)->mutex.mtx);
INIT_FAIL_2:
        pthread_mutexattr_destroy(&attr);
INIT_FAIL_1:
        free(*ccr);
INIT_SUCCESS:
        return err;
    }

    return EINVAL;
}

int ccr_exec(ccr_s *ccr, condition_func cond, void *cond_param,
             cs_body_func body, void *body_param)
{
    int err = EINVAL;

    if (ccr && cond) {
        BSEM_WAIT(ccr->mutex);

        while (!cond(cond_param)) {
            ccr->n1++;

            if (ccr->n2 > 0) {
                ccr->n2--;
                BSEM_POST(ccr->q2);
            } else {
                BSEM_POST(ccr->mutex);
            }

            BSEM_WAIT(ccr->q1);

            ccr->n2++;

            if (ccr->n1 > 0) {
                ccr->n1--;
                BSEM_POST(ccr->q1);
            } else {
                ccr->n2--;
                BSEM_POST(ccr->q2);
            }

            BSEM_WAIT(ccr->q2);
        }

        if (body)
            body(body_param);

        if (ccr->n1 > 0) {
            ccr->n1--;
            BSEM_POST(ccr->q1);
        } else if (ccr->n2 > 0) {
            ccr->n2--;
            BSEM_POST(ccr->q2);
        } else {
            BSEM_POST(ccr->mutex);
        }
    }

RET:
    return err;
}

void ccr_destroy(ccr_s *ccr)
{
    if (ccr) {
        pthread_cond_destroy(&ccr->q2.cond);
        pthread_cond_destroy(&ccr->q1.cond);
        pthread_cond_destroy(&ccr->mutex.cond);
        pthread_mutex_destroy(&ccr->q2.mtx);
        pthread_mutex_destroy(&ccr->q1.mtx);
        pthread_mutex_destroy(&ccr->mutex.mtx);

        free(ccr);
    }
}
