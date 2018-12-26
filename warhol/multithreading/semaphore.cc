// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/multithreading/semaphore.h"

namespace warhol {

void Semaphore::Wait() {
  std::unique_lock<std::mutex> lock(mutex_);
  // This while will guard against spurious wake ups.
  while (!count_) {
    // The condition variable takes ownership of the lock. During this
    // command, the lock is freed.
    threads_waiting_++;
    condition_.wait(lock);
    // At this point the lock is acquired again.
  }
  threads_waiting_--;
  count_--;
}

void Semaphore::Notify() {
  std::lock_guard<std::mutex> lock(mutex_);
  count_++;
  condition_.notify_one();
}

void Semaphore::NotifyAll() {
  std::lock_guard<std::mutex> lock(mutex_);
  count_ += threads_waiting_;
  condition_.notify_all();
}

}  // namespace warhol
