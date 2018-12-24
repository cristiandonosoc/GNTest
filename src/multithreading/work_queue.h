// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <atomic>
#include <mutex>

#include "src/multithreading/semaphore.h"
#include "src/utils/macros.h"

namespace warhol {

// Task ------------------------------------------------------------------------

struct Task {
  using TaskFunc = void (*)(void*);

  TaskFunc task_func;
  void* user_data;
};

inline bool IsValidTask(const Task& task) { return task.task_func != nullptr; }

// WorkQueue -------------------------------------------------------------------

struct WorkQueue {
  static constexpr size_t kMaxTasks = 256;

  Task tasks[kMaxTasks];
  std::atomic<int> next_task_index = 0;
  std::atomic<int> last_task_index = 0;
  std::atomic<int> completed_tasks = 0;

  Semaphore semaphore;
  std::mutex mutex;

  DELETE_COPY_AND_ASSIGN(WorkQueue);
};

void PushTask(WorkQueue*, Task);

// Will return an empty (IsValidTask == false) task if there are no pending
// tasks.
Task DequeueTask(WorkQueue*);

// Will work on tasks as long as they're available. Returns when the thread
// could not find any more tasks to do within the queue.
void GoDoWork(WorkQueue*);

// Marks the task as done within the queue when finished.
void DoTask(WorkQueue*, const Task& task);

inline bool AllTasksCompleted(const WorkQueue& queue) {
  return queue.completed_tasks == queue.last_task_index;
}

}  // namespace warhol
