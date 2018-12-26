// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <condition_variable>
#include <mutex>

namespace warhol {

class Semaphore {
  public:
    // If there are no notifications waiting, the thread will wait on some other
    // thread to call on Notify.
    void Wait();

    // Will add one to the count of pending notifications. If there are threads
    // already waiting, this will wake up one of them. If no threads are
    // waiting, the next time a thread calls Wait, it won't sleep but rather
    // decrease the notification count.
    void Notify();
    void NotifyAll();

  private:
    std::mutex mutex_;
    std::condition_variable condition_;

    // Tracks how many notifications are currently "waiting".
    uint32_t count_ = 0;
    uint32_t threads_waiting_ = 0;
};

}  // namespace warhol
