// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/debug/timer.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <utility>

#include "warhol/debug/time_logger.h"
#include "warhol/sdl2/def.h"

namespace warhol {

float CalculateTiming(uint64_t start, uint64_t end) {
  static uint64_t freq = SDL_GetPerformanceFrequency();
  float seconds = (float)((double)(end - start) / freq);
  return seconds * 1000.0f;
}

static thread_local uint64_t event_id = 1;

Timer::Timer(Type type) : type_(type) {}
Timer::~Timer() {
  if (type_ == Type::kNone)
    return;
  End();
}
Timer::Timer(Timer&& other) {
  *this = std::move(other);
}
Timer& Timer::operator=(Timer&& other) {
  // Copy all the values from the other.
  memcpy(this, &other, sizeof(Timer));
  // Zero out the other one.
  memset(&other, 0, sizeof(Timer));
  return *this;
}

Timer Timer::ManualTimer() {
  Timer timer(Type::kManual);
  timer.Init();
  return timer;
}

Timer Timer::LoggingTimer(const char* file, int line, const char* description) {
  Timer timer(Type::kLog);
  timer.id_ = event_id++;
  timer.Init();

  TimeLogger::Event::Description event = {};
  event.file = file;
  event.line = line;
  event.description = description;
  TimeLogger::Get().StartEvent(timer.id_, std::move(event));
  return timer;
}

void Timer::Init() {
  assert(!init_);
  switch (type_) {
    case Type::kNone:
      assert(false);
    case Type::kManual:
      start_ = SDL_GetPerformanceCounter();
      break;
    case Type::kLog:
      // Logging timers initialize logic is in the factory.
      break;
  }
  init_ = true;
}

float Timer::End() {
  assert(init_);
  switch (type_) {
    case Type::kNone: assert(false);
    case Type::kManual:
      return CalculateTiming(start_, SDL_GetPerformanceCounter());
    case Type::kLog: {
      if (id_ == 0)
        break;
      // Log the event and mark the timer as done.
      TimeLogger::Get().EndEvent(id_);
      id_ = 0;
      break;
    }
  }

  return 0;
}

}  // namespace warhol
