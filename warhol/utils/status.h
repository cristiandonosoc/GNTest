// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>
#include <ostream>

#include "warhol/utils/macros.h"
#include "warhol/utils/log.h"

namespace warhol {

class Status {
 public:
  static Status Ok() { return Status(); }

  Status();
  Status(LogLevel, std::string err_msg);

  explicit Status(std::string err_msg);          // Will set error status.
  explicit Status(const char* fmt, ...)
      PRINTF_FORMAT(2, 3);  // Will set error status.
  explicit Status(const char* file, int line, const char* fmt, ...)
      PRINTF_FORMAT(4, 5);

  bool ok() const { return err_msg_.empty(); }

  const char* file() const { return file_; }
  int line() const { return line_; }

  const std::string& err_msg() const { return err_msg_; }
  LogLevel level() const { return level_; }

 private:
  const char* file_;
  int line_;
  LogLevel level_ = LOG_ERROR;
  std::string err_msg_;
};

std::ostream& operator<<(std::ostream&, const Status&);

#define STATUS(fmt) \
  Status(__FILE__, __LINE__, fmt)
#define STATUS_VA(fmt, ...) \
  Status(__FILE__, __LINE__, fmt, __VA_ARGS__)

#define LOG_STATUS(status) \
  ::warhol::LogEntry().stream() << status

}  // namespace warhol
