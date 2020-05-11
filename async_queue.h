#ifndef GATEWAY_ASYNC_QUEUE_H_
#define GATEWAY_ASYNC_QUEUE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(WEBRTC_WIN)
#include <windows.h>
#elif defined(WEBRTC_POSIX)
#include <pthread.h>
#else
#error "Must define either WEBRTC_WIN or WEBRTC_POSIX."
#endif

typedef struct async_queue_element async_queue_element;
struct async_queue_element {
	void* data;
	async_queue_element *next;
	/*memset the data not release*/
	void (* data_clean)(void* data);
};

struct async_queue;
typedef struct async_queue_element_pool async_queue_element_pool;
struct async_queue_element_pool {
	async_queue* instance;	
	async_queue_element *sqi_list;
	async_queue_element *sqi_freelist;
	int max_sqi_items;
	bool created;
	bool f_alloc;//first alloc
	void (* elements_data_new)(async_queue_element_pool* pool);//new
	void (* elements_data_delete)(async_queue_element_pool* pool);//free
	pthread_mutex_t sqi_freelist_lock;
};

/*Allocate a bunch of items 
 *at once to reduce fragmentation 
 * Link together all the new items except the first one
 * (which we'll return to the caller) for placement on
 * the freelist.
 */ 
void async_queue_element_pool_create(
	async_queue_element_pool* pool,int items,
	void (*elements_data_new)(async_queue_element_pool *),
	void (*elements_data_free)(async_queue_element_pool *));
void async_queue_element_pool_destory(async_queue_element_pool* pool);

/*new element from pool*/
async_queue_element *async_queue_element_new(async_queue_element_pool* pool);
/*free element and link it to pool list*/
void async_queue_element_free(async_queue_element_pool* pool,async_queue_element *element);

typedef struct async_queue async_queue;
struct async_queue {
	async_queue_element *head;
	async_queue_element *tail;
	async_queue_element_pool* pool;
	bool enable;
	//int capacity;
	int length;
	bool wait;//true for blocking
	pthread_cond_t cond;
	pthread_mutex_t lock;
};

void async_queue_init(async_queue *queue,bool wait);
void async_queue_init(async_queue *queue,bool wait,int pool_capacity,
		void (*elements_data_new)(async_queue_element_pool *),
		void (*elements_data_free)(async_queue_element_pool *));
void async_queue_clean(async_queue *queue);
void async_queue_enable(async_queue *queue,bool enable);
bool async_queue_is_enable(async_queue *queue);
int async_queue_length(async_queue *queue);
//bool async_queue_full(async_queue *queue);
async_queue_element *async_queue_pop(async_queue *queue);
async_queue_element *async_queue_pop_wait(async_queue *queue,int give_up_after_ms);
void async_queue_push(async_queue *queue, async_queue_element *element);
#endif
