// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <sstream>

namespace warhol {

using LogLevel = int;
// Remember to update kLogLevelStrings in log.cc
constexpr LogLevel LOG_DEBUG = 0;
constexpr LogLevel LOG_INFO = 1;
constexpr LogLevel LOG_WARNING = 2;
constexpr LogLevel LOG_ERROR = 3;

const char* LogLevelToString(LogLevel);

class LogEntry {
 public:
  LogEntry();
  LogEntry(LogLevel);
  LogEntry(LogLevel, const char* file, int line);
  ~LogEntry();

  std::ostream& stream() { return os_; }

 private:
  std::ostringstream os_;
};

#define LOG(level) \
  ::warhol::LogEntry(::warhol::LOG_##level, __FILE__, __LINE__).stream()

}  // namespace warhol

