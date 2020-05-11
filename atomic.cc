
/* We are not permitted to call into any GLib functions from here, so we
 * can not use GMutex.
 *
 * Fortunately, we already take care of the Windows case above, and all
 * non-Windows platforms on which glib runs have pthreads.  Use those.
 */
#include <pthread.h>
#include <stdint.h>
#include "atomic.h"

#include "rtc_base/atomic_ops.h"

#if 0
static pthread_mutex_t atomic_lock = PTHREAD_MUTEX_INITIALIZER;

int atomic_int_get(const volatile int *atomic)
{
  int value;
  pthread_mutex_lock (&atomic_lock);
  value = *atomic;
  pthread_mutex_unlock (&atomic_lock);

  return value;
}

void atomic_int_set (volatile int *atomic,
                    int           value)
{
  pthread_mutex_lock (&atomic_lock);
  *atomic = value;
  pthread_mutex_unlock (&atomic_lock);
}

void atomic_int_inc(volatile int *atomic)
{
  pthread_mutex_lock (&atomic_lock);
  (*atomic)++;
  pthread_mutex_unlock (&atomic_lock);
}

bool atomic_int_dec_and_test (volatile int *atomic)
{
  bool is_zero;

  pthread_mutex_lock (&atomic_lock);
  is_zero = --(*atomic) == 0;
  pthread_mutex_unlock (&atomic_lock);

  return is_zero;
}


bool atomic_int_compare_and_exchange(volatile int *atomic,
                                     int           oldval,
                                     int           newval)
{
  bool success;

  pthread_mutex_lock (&atomic_lock);

  if ((success = (*atomic == oldval)))
    *atomic = newval;

  pthread_mutex_unlock (&atomic_lock);

  return success;
}

int atomic_int_add (volatile int *atomic,
                    int           val)
{
  int oldval;

  pthread_mutex_lock (&atomic_lock);
  oldval = *atomic;
  *atomic = oldval + val;
  pthread_mutex_unlock (&atomic_lock);

  return oldval;
}

unsigned int atomic_int_and(volatile unsigned int *atomic,
                    unsigned int val)
{
  unsigned int oldval;

  pthread_mutex_lock (&atomic_lock);
  oldval = *atomic;
  *atomic = oldval & val;
  pthread_mutex_unlock (&atomic_lock);

  return oldval;
}

unsigned int atomic_int_or (volatile unsigned int *atomic,
                   unsigned int           val)
{
  unsigned int oldval;

  pthread_mutex_lock (&atomic_lock);
  oldval = *atomic;
  *atomic = oldval | val;
  pthread_mutex_unlock (&atomic_lock);

  return oldval;
}

unsigned int atomic_int_xor(
  volatile unsigned int *atomic,
  unsigned int           val)
{
  unsigned int oldval;

  pthread_mutex_lock (&atomic_lock);
  oldval = *atomic;
  *atomic = oldval ^ val;
  pthread_mutex_unlock (&atomic_lock);

  return oldval;
}

int
atomic_int_exchange_and_add (volatile int *atomic,
                               int           val)
{
  return (atomic_int_add) (atomic, val);
}
#else
int atomic_int_get(const volatile int *atomic)
{
	return rtc::AtomicOps::AcquireLoad(atomic);
}

void atomic_int_set (volatile int *atomic,
                     int           value)
{
	rtc::AtomicOps::ReleaseStore(atomic, value);
}

void atomic_int_inc(volatile int *atomic)
{
	rtc::AtomicOps::Increment(atomic);
}

bool atomic_int_dec_and_test (volatile int *atomic)
{
	return (rtc::AtomicOps::Decrement(atomic) == 0);
}


bool atomic_int_compare_and_exchange(volatile int *atomic,
                                     int           oldval,
                                     int           newval)
{
	return (rtc::AtomicOps::CompareAndSwap(atomic,oldval,newval) == 0);
}

#if 0

int atomic_int_add (volatile int *atomic,
                    int           val)
{
	
}
unsigned int atomic_int_and(volatile unsigned int *atomic,
                    unsigned int val)
{

}

unsigned int atomic_int_or (volatile unsigned int *atomic,
                   unsigned int           val)
{

}

unsigned int atomic_int_xor(
  volatile unsigned int *atomic,
  unsigned int           val)
{

}

int
atomic_int_exchange_and_add (volatile int *atomic,
                               int           val)
{

}
#endif

#endif
