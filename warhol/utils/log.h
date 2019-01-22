// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <sstream>

#include "warhol/utils/location.h"

namespace warhol {

// Remember to update kLogLevelStrings in log.cc
/* using LogLevel = int; */
/* constexpr LogLevel LOG_DEBUG = 0; */
/* constexpr LogLevel LOG_INFO = 1; */
/* constexpr LogLevel LOG_WARNING = 2; */
/* constexpr LogLevel LOG_ERROR = 3; */

enum class LogLevel {
  kDEBUG,
  kINFO,
  kWARNING,
  kERROR,
  kNO_FRAME,
};

const char* LogLevelToString(LogLevel);

class LogEntry {
 public:
  LogEntry(LogLevel);
  LogEntry(LogLevel, const Location&);
  ~LogEntry();

  std::ostream& stream() { return os_; }

 private:
  std::ostringstream os_;
};

#define LOG(level) ::warhol::LogEntry(::warhol::LogLevel::k##level, FROM_HERE).stream()

}  // namespace warhol

