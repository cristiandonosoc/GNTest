// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/platform/timing.h"

#include "warhol/platform/platform.h"
#include "warhol/utils/log.h"

namespace warhol {

void PlatformUpdateTiming(PlatformTime* time) {
  static uint64_t initial_time = GetNanoseconds();

  // Get the current time.
  uint64_t current_time = GetNanoseconds() - initial_time;

  auto total_time = time->total_time;
  time->frame_delta =
      (float)(total_time > 0 ? (double)(current_time - total_time)
                             : (1.0 / 60.0));
  time->frame_delta /= 1000000000;
  time->frame_deltas[time->frame_deltas_index++] = time->frame_delta;
  if (time->frame_deltas_index >= PlatformTime::kFrameTimesCounts)
    time->frame_deltas_index = 0;


  time->total_time = current_time;
  time->seconds = (float)((float)time->total_time / 1000000000.0f);

  static uint64_t total_samples = 0;
  total_samples++;

  // Calculate the rolling average.
  float accum = 0;
  for (int i = 0; i < PlatformTime::kFrameTimesCounts; i++) {
    accum += time->frame_deltas[i];
  }
  accum /= total_samples < PlatformTime::kFrameTimesCounts
               ? total_samples
               : PlatformTime::kFrameTimesCounts;
  time->frame_delta_average = accum;
  time->frame_rate = 1.0f / accum;
}

}  // namespace
