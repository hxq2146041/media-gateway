
#include "transport.h"
#include "websockets_transport.h"
#include "absl/memory/memory.h"
#include "rtc_base/checks.h"

static void transport_message_clean(void *data) 
{
	if (data != NULL) {
		transport_message* message = (transport_message*)data;
		if (message->msg != NULL) {
			free(message->msg);
			message->msg = NULL;
		}
	}
}

static void transport_message_create(async_queue_element_pool* pool)
{
	RTC_DCHECK(pool != NULL);
	if (pool->sqi_list) {
		async_queue_element *sqi_list = pool->sqi_list;
		RTC_LOG(INFO) << "max_sqi_items:" << pool->max_sqi_items;
		int i = 0;
		for (i = 0;i < pool->max_sqi_items;i++) {
			transport_message* info = 
				(transport_message*)malloc(sizeof(transport_message));
			info->msg = NULL;
			sqi_list[i].data = (void*)info;
			sqi_list[i].data_clean = &transport_message_clean;
		}		
	}
}

static void transport_message_destory(async_queue_element_pool* pool)
{
	RTC_DCHECK(pool != NULL);
	if (pool->sqi_list) {
		async_queue_element *sqi_list = pool->sqi_list;
		int i = 0;
		for (i = 0;i < pool->max_sqi_items;i++) {
			transport_message* info = 
				(transport_message*)sqi_list[i].data;
			transport_message_clean(info);
			if (info != NULL) {
				free(info);
				info = NULL;
			}
		}		
	}
}

void transport_message_queue_init(async_queue *mq,int capacity)
{
	async_queue_init(mq,false,capacity,
		&transport_message_create,
		&transport_message_destory);
	//memset(&mq->pool,0,sizeof(async_queue_element_pool));
	//mq->pool.elements_data_new = &transport_message_create;
	//mq->pool.elements_data_delete = &transport_message_destory;
}

void transport_message_queue_clean(async_queue *mq)
{
  async_queue_clean(mq);
}

async_queue_element *transport_message_new(async_queue *mq)
{
  return async_queue_element_new(mq->pool);
}

void transport_message_free(async_queue *mq,async_queue_element *item)
{
  async_queue_element_free(mq->pool,item);
}

void transport_message_push(async_queue *mq, async_queue_element *item)
{
	async_queue_push(mq,item);
}

async_queue_element* transport_message_pop(async_queue *mq)
{
  return async_queue_pop/*_wait*/(mq/*,-1*/);
}
#if 0
static void gateway_transport_session_free(const gateway_refcount *transport_ref) {
	gateway_transport_session *session = 
			gateway_refcount_containerof(transport_ref, gateway_transport_session, ref);
	/* This session can be destroyed, free all the resources */
	if(session->p_free)
		session->p_free(session->transport_p);
	free(session);
	session = NULL;
}
#endif

gateway_transport_session gateway_transport_session_create(void *transport_p, void (*p_free)(void *)) 
{
	gateway_transport_session tp = 
			new rtc::RefCountedObject<GatewayTransportSession>();
	if(tp == nullptr)
		return nullptr;
	tp->transport_p = transport_p;
	tp->p_free = p_free;
	atomic_int_set(&tp->destroyed, 0);
	gateway_mutex_init(&tp->mutex);
	return tp;
}


void gateway_transport_session_destroy(gateway_transport_session& session) {

	if(session == nullptr ||
			atomic_int_compare_and_exchange(&session->destroyed, 0, 1))
		return;
	/* This session can be destroyed, free all the resources */
	if(session->p_free)
		session->p_free(session->transport_p);
	//session->Release();
	session = nullptr;	
}

std::unique_ptr<ITransport> createTransport(
	TransportType type,ITransportCallbacks *callback)
{
	switch(type) {
		case TRANSPORT_WEBSOCKET:
			return absl::make_unique<WebsocketsTransport>(callback);
		default:
			break;
	};
	return nullptr;
}

