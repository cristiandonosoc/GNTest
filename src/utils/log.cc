// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <iostream>

#include "utils/log.h"

namespace warhol {

namespace {

const char* kLogLevelStrings[] = {"DEBUG", "INFO", "WARNING", "ERROR"};

}  // namespace

const char*
LogLevelToString(LogLevel level) {
  return kLogLevelStrings[(int)level];
}

LogEntry::LogEntry() = default;

LogEntry::LogEntry(LogLevel level) {
  os_ << "[" << LogLevelToString(level) << "] ";
}

LogEntry::LogEntry(LogLevel level, const char* file, int line) {
  os_ << "[" << LogLevelToString(level) << "]"
      << "[" << file << ":" << line << "] ";
}

LogEntry::~LogEntry() {
  os_ << std::endl;
  std::cerr << os_.str();
  std::cerr.flush();
}

}  // namespace warhol
