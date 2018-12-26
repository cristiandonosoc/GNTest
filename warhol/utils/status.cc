// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/utils/status.h"

#include <assert.h>
#include <stdio.h>

#include "warhol/utils/string.h"

namespace warhol {

Status::Status() = default;

Status::Status(std::string err_msg) : err_msg_(std::move(err_msg)) {}

Status::Status(LogLevel level, std::string err_msg)
    : level_(level), err_msg_(std::move(err_msg)) {}

Status::Status(const char* fmt, ...) {
  va_list va;
  va_start(va, fmt);
  err_msg_ = StringPrintfV(fmt, va);
  va_end(va);
}

Status::Status(const char* file, int line, const char* fmt, ...)
    : file_(file), line_(line) {
  va_list va;
  va_start(va, fmt);
  err_msg_ = StringPrintfV(fmt, va);
  va_end(va);
}

std::ostream&
operator<<(std::ostream& os, const Status& status) {
  os << "[" << LogLevelToString(status.level()) << "]"
     << "[" << status.file() << ":" << status.line() << "] "
     << status.err_msg() << ". ";
  return os;
}

}  // namespace warhol
