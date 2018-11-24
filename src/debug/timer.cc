// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/debug/timer.h"

#include <stdio.h>

#include <utility>

#include "src/debug/time_logger.h"
#include "src/sdl2/def.h"

namespace warhol {


static thread_local uint64_t event_id = 1;

Timer::Timer(const char* file, int line, const char* description)
    : id_(event_id++) {

  TimeLogger::Event::Description event = {};
  event.file = file;
  event.line = line;
  event.description = description;
  TimeLogger::Get().StartEvent(id_, std::move(event));
}

Timer::~Timer() {
  if (id_ == 0)
    return;
  End();
}

void Timer::End() {
  TimeLogger::Get().EndEvent(id_);
  id_ = 0;
}

}  // namespace warhol
