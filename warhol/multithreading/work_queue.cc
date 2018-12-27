// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/multithreading/work_queue.h"

#include "warhol/utils/log.h"

#include <thread>

namespace warhol {

void PushTask(WorkQueue* queue, Task task) {
  std::lock_guard<std::mutex> lock(queue->mutex);
  queue->tasks[queue->last_task_index % WorkQueue::kMaxTasks] = std::move(task);

  // No instructions can be reordered before and after this fence.
  std::atomic_signal_fence(std::memory_order_acq_rel);

  // We publish the result out.
  queue->last_task_index++;
  queue->semaphore.Notify();
}

Task DequeueTask(WorkQueue* queue) {
  // We store the index of the task we want to dequeue.
  int current_index = queue->next_task_index;
  if (current_index < queue->last_task_index) {
    // This compare & exchange will only change |next_task_index| if at this
    // point no other thread has change it before. ie: If |next_task_index|
    // is still equal to |current_index| at this point, add 1 to it.
    // This returns whether there was an exchange to be done.
    bool exchanged = queue->next_task_index.compare_exchange_strong(
        current_index, current_index + 1);

    if (!exchanged) {
      // Someone else beat us to this task. We continue to check for a next
      // possible one.
      return {};
    }

    // No reads before this fence.
    std::atomic_signal_fence(std::memory_order_acq_rel);

    int i = current_index % WorkQueue::kMaxTasks;
    return queue->tasks[i];
  }
  return {};
}

void GoDoWork(WorkQueue* queue) {
  Task task = DequeueTask(queue);
  while (IsValidTask(task)) {
    DoTask(queue, task);
    task = DequeueTask(queue);
  }
}

void DoTask(WorkQueue* queue, const Task& task) {
  task.task_func(task.user_data);
  queue->completed_tasks++;
}


}  // namespace warhol
