// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/platform/timing.h"

#include <chrono>

#include "warhol/platform/platform.h"

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

Timepoint GetCurrentTime() {
  auto now = std::chrono::system_clock::now();
  auto seconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
  auto fraction = now - seconds;

  time_t cnow = std::chrono::system_clock::to_time_t(now);
  std::tm* tm = std::localtime(&cnow);
  Timepoint time = {};
  time.hours = tm->tm_hour;
  time.minutes = tm->tm_min;
  time.seconds = tm->tm_sec;
  time.ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(fraction).count();
  return time;
}

std::string TimeToString(const Timepoint& time) {
  char buf[30];
  snprintf(buf, sizeof(buf) - 1, "%02d:%02d:%02d.%03d",
           time.hours, time.minutes, time.seconds, time.ms);
  return buf;
}

}  // namespace
