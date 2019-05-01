// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <sstream>

#include "warhol/utils/location.h"
#include "warhol/utils/macros.h"

namespace warhol {

enum class LogLevel {
  kDEBUG,
  kINFO,
  kWARNING,
  kERROR,
  kASSERT,
  kNO_FRAME,
};

const char* LogLevelToString(LogLevel);

class LogEntry {
 public:
  LogEntry(LogLevel);
  LogEntry(LogLevel, const Location&);

  // Creates a log entry that will print the assert message and die.
  LogEntry(const Location&, bool condition, const char* condition_str);
  ~LogEntry();

  DELETE_COPY_AND_ASSIGN(LogEntry);
  DEFAULT_MOVE_AND_ASSIGN(LogEntry);

  std::ostream& stream() { return os_; }

  void set_assert(bool a) { assert_ = a; }
  void set_condition(const char* c) { condition_ = c; }
  void set_location(Location l) { location_ = std::move(l); }

 private:
  LogLevel level_ = LogLevel::kINFO;
  std::ostringstream os_;
  bool assert_ = false;
  const char* condition_ = nullptr;
  Location location_ = {};
};

// This class is meant to look as a ostream, but ignore all input.
class NullStream : std::ostream, std::streambuf {
 public:
  NullStream() : std::ostream(this) {}

  // All operations are no-ops.
  int overflow(int) { return 0; }
  std::ostream& stream() { return *this; }
};

#define LOG(level) \
  ::warhol::LogEntry(::warhol::LogLevel::k##level, FROM_HERE).stream()

// Asserts ---------------------------------------------------------------------

#ifdef DEBUG_MODE

#define ASSERT(condition) \
  ::warhol::LogEntry(FROM_HERE, !(condition), #condition).stream()

#define NOT_IMPLEMENTED() \
  ::warhol::LogEntry(FROM_HERE, true, "Not implemented").stream()

#define NOT_REACHED() \
  ::warhol::LogEntry(FROM_HERE, true, "Invalid Path").stream()

#else

#define ASSERT(condition) ::warhol::NullStream().stream()
#define NOT_IMPLEMENTED() ::warhol::NullStream().stream()
#define NOT_REACHED() ::warhol::NullStream().stream()

/* #define NOT_REACHED() do {} while (false) */

#endif




}  // namespace warhol

