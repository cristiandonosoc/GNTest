// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/utils/log.h"

#include <iostream>

#include "warhol/utils/macros.h"

namespace warhol {

namespace {

void AssertionFailed(Location loc,
                     const char* condition,
                     std::string message) {
  printf("Assertion failed at %s (%s:%d):\n", loc.function, loc.file, loc.line);

  if (message.empty()) {
    printf("\n    %s\n\n", condition);
  } else {
    printf("\n    %s\n\n", condition);
    printf("%s\n", message.c_str());
  }

  LocationStack* stack = GetLocationStack();
  printf("Printing contextual stack of size: %d\n", stack->size);
  PrintLocationStack(stack);

  fflush(stdout);

  SEGFAULT();
}

}  // namespace

const char*
LogLevelToString(LogLevel level) {
  switch (level) {
    case LogLevel::kDEBUG: return "DEBUG";
    case LogLevel::kINFO: return "INFO";
    case LogLevel::kWARNING: return "WARNING";
    case LogLevel::kERROR: return "ERROR";
    case LogLevel::kASSERT: return "ASSERT";
    case LogLevel::kNO_FRAME: return "NO_FRAME";
  }

  NOT_REACHED() << "Unknown LogLevel.";
  return nullptr;
}

LogEntry::LogEntry(LogLevel level) : level_(level) {
  if (level == LogLevel::kNO_FRAME || level == LogLevel::kASSERT)
    return;
  os_ << "[" << LogLevelToString(level) << "] ";
}

LogEntry::LogEntry(LogLevel level, const Location& location) : level_(level) {
  if (level == LogLevel::kNO_FRAME || level == LogLevel::kASSERT)
    return;
  os_ << "[" << LogLevelToString(level) << "]"
      << "[" << location.file << ":" << location.line << "] ";
}

LogEntry::LogEntry(const Location& location,
                   bool condition,
                   const char* condition_str)
    : level_(LogLevel::kASSERT), assert_(condition) {
  if (condition) {
    set_condition(condition_str);
    set_location(location);
  }
}

// Printing --------------------------------------------------------------------

LogEntry::~LogEntry() {
  os_ << std::endl;
  if (level_ != LogLevel::kASSERT) {
    std::cerr << os_.str();
    std::cerr.flush();
  } else if (assert_) {
    AssertionFailed(location_, condition_, os_.str());
  }
}

}  // namespace warhol
