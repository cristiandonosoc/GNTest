// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/debug/time_logger.h"

#include <assert.h>

#include <map>
#include <mutex>
#include <thread>

#include "src/sdl2/def.h"
#include "src/utils/log.h"
#include "src/utils/string.h"

namespace warhol {


// We create one time logger per-thread.
TimeLogger& TimeLogger::Get() {
  thread_local TimeLogger time_logger;
  return time_logger;
}

TimeLogger::TimeLogger() {
  thread_id_ = std::hash<std::thread::id>{}(std::this_thread::get_id());
  TimeLoggerManager::Get().SuscribeTimeLogger(this);
}

TimeLogger::~TimeLogger() {
  TimeLoggerManager::Get().UnsuscribeTimeLogger(this);
}

void TimeLogger::StartEvent(uint64_t id, TimeLogger::Event::Description desc) {
  Event event = {};
  event.id = id;
  event.counter = SDL_GetPerformanceCounter();
  event.type = Event::Type::kStart;
  event.description = std::move(desc);
  events_[event_count_++] = std::move(event);
  assert(event_count_ < kMaxEventQueue);
}

void TimeLogger::EndEvent(uint64_t id) {
  Event event = {};
  event.id = id;
  event.counter = SDL_GetPerformanceCounter();
  event.type = Event::Type::kEnd;
  events_[event_count_++] = std::move(event);
  assert(event_count_ < kMaxEventQueue);
}

void
TimeLogger::LogFrame() {
  struct LogEntry {
    int indent = 0;
    uint64_t start = 0;
    uint64_t end = 0;
    float time = 0;
    Event::Description description;
  };

  size_t entry_count = 0;
  LogEntry entries[kMaxEventQueue / 2];

  int indent = 0;
  std::map<uint64_t, LogEntry*> started_events;
  for (size_t i = 0; i < event_count_; i++) {
    Event& event = events_[i];
    if (event.type == Event::Type::kStart) {
      LogEntry entry = {};
      entry.indent = indent;
      entry.start = event.counter;
      entry.description = std::move(event.description);
      entries[entry_count] = std::move(entry);

      started_events[event.id] = entries + entry_count;
      entry_count++;
      indent += 2;
    } else if (event.type == Event::Type::kEnd) {
      auto it = started_events.find(event.id);
      assert(it != started_events.end());

      LogEntry* entry = it->second;
      entry->end = event.counter;

      entry->time = CalculateTiming(entry->start, entry->end);
      indent -= 2;
    } else {
      assert(false);
    }
  }

  for (size_t i = 0; i < entry_count; i++) {
    LogEntry& entry = entries[i];
    printf("%s\n",
           StringPrintf("[%lx][%s:%d]%*c%s: %f ms",
                        thread_id(),
                        entry.description.file,
                        entry.description.line,
                        entry.indent,
                        ' ',
                        entry.description.description,
                        entry.time)
               .data());
  }

  fflush(stdout);
  event_count_ = 0;
}

// TimeLoggerManager -----------------------------------------------------------

TimeLoggerManager::TimeLoggerManager() = default;

TimeLoggerManager& TimeLoggerManager::Get() {
  static TimeLoggerManager manager;
  return manager;
}

std::mutex kTimeLoggerManagerMutex;

void TimeLoggerManager::SuscribeTimeLogger(TimeLogger* logger) {
  LOG(DEBUG) << "Suscribing logger for thread: " << logger->thread_id();
  std::lock_guard<std::mutex> lock(kTimeLoggerManagerMutex);
  size_t i = 0;
  for (; i < TimeLoggerManager::kMaxTimeLoggers; i++) {
    if (!loggers_[i])
      break;
  }

  if (i == TimeLoggerManager::kMaxTimeLoggers) {
    LOG(ERROR) << "Could not find a free time logger slot!";
    exit(1);
  }

  loggers_[i] = logger;
}

void TimeLoggerManager::UnsuscribeTimeLogger(TimeLogger* logger) {
  std::lock_guard<std::mutex> lock(kTimeLoggerManagerMutex);

  size_t i = 0;
  for (; i < TimeLoggerManager::kMaxTimeLoggers; i++) {
    if (loggers_[i] == logger)
      break;
  }

  if (i == TimeLoggerManager::kMaxTimeLoggers) {
    LOG(ERROR) << "Could not find suscribed logger!";
    exit(1);
  }

  loggers_[i] = nullptr;
}

void TimeLoggerManager::LogFrame() {
  // TODO(Cristian): Do a lock-free sync of this!
  for (size_t i = 0; i < kMaxTimeLoggers; i++) {
    if (!loggers_[i])
      continue;

    loggers_[i]->LogFrame();
  }
}


}  // namespace warhol
