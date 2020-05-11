
#ifndef GATEWAY_MUTEX_H
#define GATEWAY_MUTEX_H

#include <pthread.h>
#include <errno.h>

/*! \brief Gateway mutex implementation */
typedef pthread_mutex_t gateway_mutex;
/*! \brief Gateway mutex initialization */
#define gateway_mutex_init(a) pthread_mutex_init(a,NULL)
/*! \brief Gateway static mutex initializer */
#define GATEWAY_MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER
/*! \brief Gateway mutex destruction */
#define gateway_mutex_destroy(a) pthread_mutex_destroy(a)
/*! \brief Gateway mutex lock wrapper (selective locking debug) */
#define gateway_mutex_lock(a) pthread_mutex_lock(a);
/*! \brief Gateway mutex unlock wrapper (selective locking debug) */
#define gateway_mutex_unlock(a) pthread_mutex_unlock(a);

/*! \brief Gateway condition implementation */
typedef pthread_cond_t gateway_condition;
/*! \brief Gateway condition initialization */
#define gateway_condition_init(a) pthread_cond_init(a,NULL)
/*! \brief Gateway condition destruction */
#define gateway_condition_destroy(a) pthread_cond_destroy(a)
/*! \brief Gateway condition wait */
#define gateway_condition_wait(a, b) pthread_cond_wait(a, b);
/*! \brief Gateway condition timed wait */
#define gateway_condition_timedwait(a, b, c) pthread_cond_timedwait(a, b, c);
/*! \brief Gateway condition signal */
#define gateway_condition_signal(a) pthread_cond_signal(a);
/*! \brief Gateway condition broadcast */
#define gateway_condition_broadcast(a) pthread_cond_broadcast(a);

#endif
