// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdlib.h>

#include "warhol/debug/timer.h"
#include "warhol/utils/macros.h"

namespace warhol {


// TimeLogger are associated with a thread, and will suscribe and unsuscribe
// with the the TimeLoggerManager on construction/destruction.
class TimeLogger {
 public:
  struct Event {
    enum Type {
      kStart,
      kEnd,
      kNone,
    };
    struct Description {
      const char* file;
      const char* description;
      int line;
    };

    uint64_t id = 0;
    uint64_t counter = 0;
    Type type = Type::kNone;
    // Only used on start events.
    Description description = {};
  };


  TimeLogger();
  ~TimeLogger();

  bool Init();
  DELETE_COPY_AND_ASSIGN(TimeLogger);
  DELETE_MOVE_AND_ASSIGN(TimeLogger);

  static TimeLogger& Get();
  size_t thread_id() const { return thread_id_; }
  void LogFrame();

  void StartEvent(uint64_t id, Event::Description event);
  void EndEvent(uint64_t id);

 private:
  static constexpr size_t kMaxEventQueue = 4096;
  Event events_[kMaxEventQueue];
  size_t event_count_ = 0;
  size_t thread_id_;
};

class TimeLoggerManager {
 public:
  TimeLoggerManager();
  DELETE_COPY_AND_ASSIGN(TimeLoggerManager);
  DELETE_MOVE_AND_ASSIGN(TimeLoggerManager);

  static TimeLoggerManager& Get();

  void SuscribeTimeLogger(TimeLogger*);
  void UnsuscribeTimeLogger(TimeLogger*);

  void LogFrame();

 private:
  static constexpr size_t kMaxTimeLoggers = 4;
  TimeLogger* loggers_[kMaxTimeLoggers];
};

}  // namespace warhol
