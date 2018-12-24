// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <stdio.h>

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>

#include <debug/timer.h>
#include <utils/log.h>
#include <voxel_terrain.h>
#include <voxel_utils.h>

#include <multithreading/work_queue.h>

using namespace warhol;

void ThreadFunc(WorkQueue* queue) {
  while (true) {
    GoDoWork(queue);
    // We wait until more work is present.
    queue->semaphore.Wait();
  }
}

void PrintStringTask(void *user_data) {
  const char* string = (const char*)user_data;
  LOG(DEBUG) << "TASK: " << string;
}

int main() {
  LOG(INFO) << "Starting";

  WorkQueue queue = {};

  constexpr int kThreadCount = 10;
  std::thread threads[kThreadCount];
  for (size_t i = 0; i < kThreadCount; i++)
    threads[i] = std::thread(ThreadFunc, &queue);

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

  /* auto timer = Timer::ManualTimer(); */
  /* timer.Init(); */

  /* VoxelTerrain terrain(nullptr); */
  /* if (!terrain.Init()) { */
  /*   LOG(ERROR) << "Could not initialize terrain."; */
  /*   exit(1); */
  /* } */

  /* SetupSphere(&terrain, {}, 10); */

  /* auto time = timer.End(); */
  /* printf("Timing: %.3f ms", time); */
}
