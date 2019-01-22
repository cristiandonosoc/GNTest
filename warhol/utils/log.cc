// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/utils/log.h"

#include <iostream>

#include "warhol/utils/assert.h"

namespace warhol {

const char*
LogLevelToString(LogLevel level) {
  switch (level) {
    case LogLevel::kDEBUG: return "kDEBUG";
    case LogLevel::kINFO: return "kINFO";
    case LogLevel::kWARNING: return "kWARNING";
    case LogLevel::kERROR: return "kERROR";
    case LogLevel::kNO_FRAME: return "kNO_FRAME";
  }

  NOT_REACHED("Unknown LogLevel.");
  return nullptr;
}

LogEntry::LogEntry(LogLevel level) {
  if (level == LogLevel::kNO_FRAME)
    return;
  os_ << "[" << LogLevelToString(level) << "] ";
}

LogEntry::LogEntry(LogLevel level, const Location& location) {
  if (level == LogLevel::kNO_FRAME)
    return;
  os_ << "[" << LogLevelToString(level) << "]"
      << "[" << location.file << ":" << location.line << "] ";
}

LogEntry::~LogEntry() {
  os_ << std::endl;
  std::cerr << os_.str();
  std::cerr.flush();
}

}  // namespace warhol
