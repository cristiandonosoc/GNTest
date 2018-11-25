// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include "src/utils/clear_on_move.h"
#include "src/utils/macros.h"

namespace warhol {

// Macros for logging.
#define NAMED_TIMER(name, description) \
  ::warhol::Timer name =               \
      ::warhol::Timer::LoggingTimer(__FILE__, __LINE__, description)

#define TIMER(description) \
  NAMED_TIMER(__timer##__LINE_##__, description)

#define FUNCTION_TIMER() TIMER(__PRETTY_FUNCTION__)

// Returns the ms in between.
float CalculateTiming(uint64_t start, uint64_t end);

// The class timer queries the timing on Init and End.
// It's behavior is determined by what type of Timer it is.
class Timer {
 public:
   enum class Type {
     // Typically mean that this timer was moved.
     kNone = 0,
     // Manual means that Init and End have to be called manually. End will
     // return the amount of milliseconds it queried. This is useful for one-off
     // measurements.
     //
     // For manual timer, you have to call Init() and End() manually.
     kManual,
     // Log will log the start/end events to the thread local TimeLogger. This
     // will then be coalesced at frame end and get a hierarchical view of the
     // timings. This is useful for function timing tracing.
     //
     // Normally End() will be called on destruction, but you can override by
     // calling End() manually.
     // NOTE: If the timer is log, it will *not* track timing, meaning that
     // End() will return 0.
     kLog
   };
  // Creates a timer which will be called manua

  static Timer ManualTimer();
  static Timer LoggingTimer(const char* file, int line, const char* desc);
  ~Timer();

  DELETE_COPY_AND_ASSIGN(Timer);
  Timer(Timer&&);
  Timer& operator=(Timer&&);

  // Initializes the timer.
  void Init();

  // Manual: returns the time since start until the call. Can be called multiple
  // times.
  // Logging: Will log the event with the time logger. Will work once.
  float End();

 private:
  Timer(Type);

  // ID at 0 means non-initialized or already ended.
  Type type_;
  union {
    // kManual
    uint64_t start_;
    // kLog
    uint64_t id_;
  };
  bool init_ = false;
};

}  // namespace warhol
