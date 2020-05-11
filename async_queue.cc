#include "async_queue.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "rtc_base/system/warn_current_thread_is_deadlocked.h"

#if defined(WEBRTC_WIN)
#include <windows.h>
#elif defined(WEBRTC_POSIX)
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#else
#error "Must define either WEBRTC_WIN or WEBRTC_POSIX."
#endif

#define DEFAULT_CAPACITY 16

void async_queue_element_pool_create(
	async_queue_element_pool* pool,int items,
	void (*elements_data_new)(async_queue_element_pool *),
	void (*elements_data_free)(async_queue_element_pool *))
{
	if (pool == NULL 
		|| pool->instance == NULL 
		|| items < 0 
		|| items == 0) return ;
	if (pool->created || pool->sqi_list != NULL) 
		return;
	
	pthread_mutex_init(&pool->sqi_freelist_lock, nullptr);	
	pool->sqi_freelist = NULL;
	pool->sqi_list=NULL;
	pool->max_sqi_items = items;
	pool->elements_data_new = elements_data_new;
	pool->elements_data_delete = elements_data_free;
	/* Allocate a bunch of items at once to reduce fragmentation */
	pool->sqi_list = 
		(async_queue_element *)malloc(sizeof(async_queue_element) * items);
	if (NULL == pool->sqi_list) {
		return;
	}
	if (pool->elements_data_new)
		pool->elements_data_new(pool);
	else {
		int i = 0;
		for (i = 0; i < items; i++)
			pool->sqi_list[i].data = NULL;		
	}
	int i = 0;
	for (i = 2; i < items; i++)
		pool->sqi_list[i - 1].next = &pool->sqi_list[i];
	pthread_mutex_lock(&pool->sqi_freelist_lock);
	pool->sqi_list[items - 1].next = pool->sqi_freelist;
	pool->sqi_freelist = &pool->sqi_list[1];
	pool->created = true;
	pool->f_alloc = true;
	pthread_mutex_unlock(&pool->sqi_freelist_lock);
}

void async_queue_element_pool_destory(async_queue_element_pool* pool)
{
	RTC_DCHECK(pool != NULL);
	if (pool->sqi_list != NULL) {
		if (pool->elements_data_delete)
			pool->elements_data_delete(pool);
		free(pool->sqi_list);
		pool->sqi_list = NULL;
		pool->sqi_freelist = NULL;
		pool->created = false;
		pool->f_alloc = false;
		pool->max_sqi_items = 0;
		pthread_mutex_destroy(&pool->sqi_freelist_lock);				
	}	
}

async_queue_element *async_queue_element_new(async_queue_element_pool* pool) {
	RTC_DCHECK(pool != NULL);
	
	if (!pool->created)
		async_queue_element_pool_create(pool,DEFAULT_CAPACITY,NULL,NULL);
		
	if (pool->f_alloc) {
		pool->f_alloc = false;
		return &pool->sqi_list[0];
	} else {
		async_queue_element *item = NULL;
		pthread_mutex_lock(&pool->sqi_freelist_lock);
		if (pool->sqi_freelist) {
			item = pool->sqi_freelist;
			pool->sqi_freelist = item->next;
		}		
		pthread_mutex_unlock(&pool->sqi_freelist_lock);
		return item;
	}
}	

void async_queue_element_free(
		async_queue_element_pool* pool,async_queue_element *item)
{
	RTC_DCHECK(pool != NULL);
	RTC_DCHECK(item != NULL);
	pthread_mutex_lock(&pool->sqi_freelist_lock);
	if (item->data != NULL && item->data_clean)
		item->data_clean(item->data);
	else
		item->data = NULL;
	item->next = pool->sqi_freelist;
	pool->sqi_freelist = item;
	pthread_mutex_unlock(&pool->sqi_freelist_lock);	
}

static timespec GetTimespec(const int milliseconds_from_now) {
  timespec ts;

  // Get the current time.
#if USE_CLOCK_GETTIME
  clock_gettime(CLOCK_MONOTONIC, &ts);
#else
  timeval tv;
  gettimeofday(&tv, nullptr);
  ts.tv_sec = tv.tv_sec;
  ts.tv_nsec = tv.tv_usec * 1000;
#endif

  // Add the specified number of milliseconds to it.
  ts.tv_sec += (milliseconds_from_now / 1000);
  ts.tv_nsec += (milliseconds_from_now % 1000) * 1000000;

  // Normalize.
  if (ts.tv_nsec >= 1000000000) {
    ts.tv_sec++;
    ts.tv_nsec -= 1000000000;
  }
  return ts;
}

/*
 * Initializes a m queue.
 */
void async_queue_init(
	async_queue *queue,bool wait) 
{
	RTC_DCHECK(queue != NULL);
	queue->head = NULL;
	queue->head = NULL;
	queue->pool = NULL;	
	queue->enable = true;
	//queue->capacity = capacity;
	queue->length = 0;
	queue->wait = wait;
	RTC_CHECK(pthread_mutex_init(&queue->lock, nullptr) == 0);
	if (wait) {
		pthread_condattr_t cond_attr;
		RTC_CHECK(pthread_condattr_init(&cond_attr) == 0);
	#if USE_CLOCK_GETTIME && !USE_PTHREAD_COND_TIMEDWAIT_MONOTONIC_NP
		RTC_CHECK(pthread_condattr_setclock(&cond_attr, CLOCK_MONOTONIC) == 0);
	#endif
		RTC_CHECK(pthread_cond_init(&queue->cond, &cond_attr) == 0);
		pthread_condattr_destroy(&cond_attr);			
	}
}

void async_queue_init(
	async_queue *queue,
	bool wait,
	int pool_capacity,
	void (*elements_data_new)(async_queue_element_pool *),
	void (*elements_data_free)(async_queue_element_pool *)) 
{
	async_queue_init(queue,wait);
	if (pool_capacity > 0) {
		queue->pool = 
				(async_queue_element_pool *) 
					malloc(sizeof(async_queue_element_pool));
		queue->pool->instance = queue;
		queue->pool->sqi_list = NULL;
		queue->pool->sqi_freelist = NULL;
		queue->pool->created = false;
		queue->pool->f_alloc = true;//first alloc
		async_queue_element_pool_create(
			queue->pool,
			pool_capacity,
			elements_data_new,
			elements_data_free);	
	}
}

void async_queue_clean(async_queue *queue)
{
	RTC_DCHECK(queue != NULL);
	pthread_mutex_lock(&queue->lock);
	queue->head = NULL;
	queue->tail = NULL;	
	queue->enable = false;
	//queue->capacity = 0;
	queue->length = 0;	
	queue->wait = false;	
	if (queue->pool != NULL) {
		async_queue_element_pool_destory(queue->pool);
		free(queue->pool);
		queue->pool = NULL;	
	}
	pthread_mutex_unlock(&queue->lock);
	
  pthread_mutex_destroy(&queue->lock);
  pthread_cond_destroy(&queue->cond);	
}

void async_queue_enable(async_queue *queue,bool enable)
{
	RTC_DCHECK(queue != NULL);
	pthread_mutex_lock(&queue->lock);
	queue->enable = enable;
	pthread_mutex_unlock(&queue->lock);
}

bool async_queue_is_enable(async_queue *queue)
{
	RTC_DCHECK(queue != NULL);
	bool enable = false;
	pthread_mutex_lock(&queue->lock);
	enable = queue->enable;
	pthread_mutex_unlock(&queue->lock);
	return enable;
}

int async_queue_length(async_queue *queue)
{
	RTC_DCHECK(queue != NULL);
	int size = 0;
	pthread_mutex_lock(&queue->lock);
	size = queue->length;
	pthread_mutex_unlock(&queue->lock);
	return size;
}

#if 0
bool async_queue_full(async_queue *queue)
{
	RTC_DCHECK(queue != NULL);
	int result = false;
	pthread_mutex_lock(&queue->lock);
	if (queue->length > queue->capacity)
		result = true;
	else
		result = false;
	pthread_mutex_unlock(&queue->lock);
	return result;
}
#endif

/*
 * Looks for an item on a connection queue, but doesn't block if there isn't
 * one.
 * Returns the item, or NULL if no item is available
 */
async_queue_element *async_queue_pop(async_queue *queue) {
	RTC_DCHECK(queue != NULL);
	async_queue_element *item;
	pthread_mutex_lock(&queue->lock);
	if (!queue->enable/* || queue->wait*/ 
		|| queue->length == 0) {
		pthread_mutex_unlock(&queue->lock);
		return NULL;
	}
	item = queue->head;
	if (NULL != item) {
	    queue->head = item->next;
	    if (NULL == queue->head)
	        queue->tail = NULL;
	}
	queue->length--;
	pthread_mutex_unlock(&queue->lock);
	return item;
}

async_queue_element* async_queue_pop_wait(async_queue *queue,int give_up_after_ms) {
	RTC_DCHECK(queue != NULL);
  // Instant when we'll stop waiting and return an error. nullopt if we should
  // never give up.
  const absl::optional<timespec> give_up_ts =
      give_up_after_ms == -1
          ? absl::nullopt
          : absl::make_optional(GetTimespec(give_up_after_ms));	
					
	async_queue_element *item = NULL;
	
	pthread_mutex_lock(&queue->lock);
	if (!queue->enable) {
		pthread_mutex_unlock(&queue->lock);
		return NULL;
	}

  // Wait for `queue->cond` to trigger and `queue->length` to be set, with the
  // given timeout (or without a timeout if none is given).
  const auto wait = [&](const absl::optional<timespec> timeout_ts) {
    int error = 0;
    while ((queue->length == 0) && error == 0) {
      if (timeout_ts == absl::nullopt) {
        error = pthread_cond_wait(&queue->cond, &queue->lock);
      } else {
#if USE_PTHREAD_COND_TIMEDWAIT_MONOTONIC_NP
        error = pthread_cond_timedwait_monotonic_np(&queue->cond, &queue->lock,
                                                    &*timeout_ts);
#else
        error =
            pthread_cond_timedwait(&queue->cond, &queue->lock, &*timeout_ts);
#endif
      }
    }
    return error;
  };
	
  int error = wait(give_up_ts);
#if 0	
	if (error == ETIMEDOUT) {
		webrtc::WarnThatTheCurrentThreadIsProbablyDeadlocked();
		pthread_mutex_unlock(&queue->lock);
		return NULL;
	}
#else
	if (error != 0) {
		webrtc::WarnThatTheCurrentThreadIsProbablyDeadlocked();
	} else {
		item = queue->head;
		if (NULL != item) {
				queue->head = item->next;
				if (NULL == queue->head)
						queue->tail = NULL;
		}
		queue->length--;
	}
#endif	
	pthread_mutex_unlock(&queue->lock);	
	return item;
}

/*
 * Adds an item to a connection queue.
 */
void async_queue_push(async_queue *queue, async_queue_element *item) {
	RTC_DCHECK(queue != NULL);
#if 0	
	if (async_queue_full(queue)) {
		RTC_LOG(WARNING) << "async_queue_push Error,async_queue has full";
		return;
	}
#endif	
	pthread_mutex_lock(&queue->lock);
	if (!queue->enable) {
		pthread_mutex_unlock(&queue->lock);
		return;
	}
	item->next = NULL;
	if (NULL == queue->tail)
		queue->head = item;
	else
		queue->tail->next = item;
	queue->tail = item;
	queue->length++;
	if (queue->wait)
		pthread_cond_broadcast(&queue->cond);
	pthread_mutex_unlock(&queue->lock);
}

