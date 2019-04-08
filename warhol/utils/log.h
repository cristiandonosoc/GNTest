// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <sstream>

#include "warhol/utils/assert.h"
#include "warhol/utils/location.h"
#include "warhol/utils/macros.h"

namespace warhol {

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

#define LOG(level) \
  ::warhol::LogEntry(::warhol::LogLevel::k##level, FROM_HERE).stream()

}  // namespace warhol

