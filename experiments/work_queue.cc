// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <stdio.h>

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>

#include <warhol/debug/timed_block.h>
#include <warhol/debug/timer.h>
#include <warhol/debug/time_logger.h>
#include <warhol/multithreading/work_queue.h>
#include <warhol/utils/log.h>

#include "voxel/voxel_terrain.h"
#include "voxel/voxel_utils.h"

using namespace warhol;

void ThreadFunc(WorkQueue* queue) {
  while (queue->running) {
    GoDoWork(queue);
    // We wait until more work is present.
    /* LOG(DEBUG) << "Thread: " << std::this_thread::get_id() << " sleeping!"; */
    queue->semaphore.Wait();
    /* LOG(DEBUG) << "Thread: " << std::this_thread::get_id() << " work up!"; */
  }
}

void PrintStringTask(void *user_data) {
  const char* string = (const char*)user_data;
  LOG(DEBUG) << "TASK: " << string;
}

int main() {
  LOG(INFO) << "Starting";

  WorkQueue queue = {};
  constexpr int kThreadCount = 0;
  std::thread threads[kThreadCount];
  for (size_t i = 0; i < kThreadCount; i++)
    threads[i] = std::thread(ThreadFunc, &queue);

#if 0

  PushTask(&queue, {PrintStringTask, (void*)"Task 0"});
  PushTask(&queue, {PrintStringTask, (void*)"Task 1"});
  PushTask(&queue, {PrintStringTask, (void*)"Task 2"});
  PushTask(&queue, {PrintStringTask, (void*)"Task 3"});
  PushTask(&queue, {PrintStringTask, (void*)"Task 4"});
  PushTask(&queue, {PrintStringTask, (void*)"Task 5"});
  PushTask(&queue, {PrintStringTask, (void*)"Task 6"});
  PushTask(&queue, {PrintStringTask, (void*)"Task 7"});
  PushTask(&queue, {PrintStringTask, (void*)"Task 8"});
  PushTask(&queue, {PrintStringTask, (void*)"Task 9"});

  GoDoWork(&queue);

  std::this_thread::sleep_for(std::chrono::seconds(1));

  PushTask(&queue, {PrintStringTask, (void*)"Task A0"});
  PushTask(&queue, {PrintStringTask, (void*)"Task A1"});
  PushTask(&queue, {PrintStringTask, (void*)"Task A2"});
  PushTask(&queue, {PrintStringTask, (void*)"Task A3"});
  PushTask(&queue, {PrintStringTask, (void*)"Task A4"});
  PushTask(&queue, {PrintStringTask, (void*)"Task A5"});
  PushTask(&queue, {PrintStringTask, (void*)"Task A6"});
  PushTask(&queue, {PrintStringTask, (void*)"Task A7"});
  PushTask(&queue, {PrintStringTask, (void*)"Task A8"});
  PushTask(&queue, {PrintStringTask, (void*)"Task A9"});

  while (!AllTasksCompleted(queue)) {
    GoDoWork(&queue);
  }

  LOG(DEBUG) << "Done with work";

  for (size_t i = 0; i < kThreadCount; i++)
    threads[i].join();

#endif

  auto timer = Timer::ManualTimer();
  timer.Init();

  {
    for (size_t  i = 0; i < 10; i++) {
    TIMED_BLOCK();

  VoxelTerrain terrain(nullptr);
  SetupSphere(&terrain, {}, 50);
  terrain.UpdateMT(&queue);

  while (!AllTasksCompleted(queue)) {
    GoDoWork(&queue);
  }
    }

  }

  auto time = timer.End();
  printf("Timing: %.3f ms\n", time);

  auto& record = TimedBlockRecord::gTimeBlockRecordArray[0];
  uint64_t cycle_hit_count = record.hit_and_cycle_count;
  uint32_t cycles = cycle_hit_count & 0xFFFFFFFF;
  uint32_t hit_count = cycle_hit_count >> 32;
  printf("[%s:%d][%s]: %u cycles, %u hits (%u cycles/hit)\n",
         record.filename,
         record.line,
         record.function,
         cycles,
         hit_count, cycles / hit_count);

  LOG(DEBUG) << "Ending queue.";
  queue.running = false;
  queue.semaphore.NotifyAll();


  for (size_t i = 0; i < kThreadCount; i++)
    threads[i].join();

  /* if (!terrain.Init()) { */
  /*   LOG(ERROR) << "Could not initialize terrain."; */
  /*   exit(1); */
  /* } */
}

