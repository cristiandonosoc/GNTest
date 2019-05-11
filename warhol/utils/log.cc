// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/utils/log.h"

#include <iostream>
#include <mutex>

#include "warhol/platform/timing.h"
#include "warhol/platform/platform.h"
#include "warhol/utils/file.h"
#include "warhol/utils/macros.h"
#include "warhol/utils/string.h"

namespace warhol {

namespace {

volatile bool kPrintToLog = true;

FileHandle CreateLogFile() {
  std::string path = StringPrintf("%s/latest.log",
                                  GetCurrentExecutableDirectory().c_str());
  return OpenFile(std::move(path));
}


void PrintToLog(const std::string& data) {
  if (kPrintToLog) {
    static FileHandle file = CreateLogFile();
    WriteToFile(&file, (void*)data.c_str(), data.size());
    Flush(&file);
  }
  std::cerr << data;
  std::cerr.flush();
}

void AssertionFailed(Location loc,
                     const char* condition,
                     std::string message) {
  auto time = GetCurrentTime();
  PrintToLog(StringPrintf("[%s] Assertion failed at %s (%s:%d):\n",
                          TimeToString(time).c_str(),
                          loc.function,
                          loc.file,
                          loc.line));

  if (message.empty()) {
    PrintToLog(StringPrintf("\n    %s\n\n", condition));
  } else {
    PrintToLog(StringPrintf("\n    %s\n\n%s\n", condition, message.c_str()));
  }

  LocationStack* stack = GetLocationStack();
  PrintToLog(StringPrintf("Printing contextual stack of size: %d\n",
             stack->size));
  PrintToLog(LocationStackToString(stack));

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
    PrintToLog(StringPrintf("[%s]", TimeToString(GetCurrentTime()).c_str()));
    PrintToLog(os_.str());
  } else if (assert_) {
    AssertionFailed(location_, condition_, os_.str());
  }
}

}  // namespace warhol
