#include "rtc_base/thread_pool.h"

namespace rtc {
std::unique_ptr<ThreadPool> CreateThreadPool(int threads) {
  return absl::make_unique<ThreadPool>(threads);
}
}
