#ifndef RTC_BASE_THREAD_POOL_H_
#define RTC_BASE_THREAD_POOL_H_

#include "absl/synchronization/internal/thread_pool.h"
#include "absl/memory/memory.h"


namespace rtc {

typedef absl::synchronization_internal::ThreadPool ThreadPool;

std::unique_ptr<ThreadPool> CreateThreadPool(int threads);

}//end rtc

#endif
