// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <assert.h>

#include <istream>

#include "log.h"
#include "status.h"
#include "string.h"

namespace warhol {

Status::Status() = default;

Status::Status(std::string err_msg)
    : type_(Type::kError), err_msg_(std::move(err_msg)) {}
Status::Status(Type type, std::string err_msg)
    : type_(type), err_msg_(std::move(err_msg)) {}

Status::Status(const char* fmt, ...) : type_(Type::kError) {
  va_list va;
  va_start(va, fmt);
  err_msg_ = StringPrintfV(fmt, va);
  va_end(va);
}

Status::Status(Type type, const char* fmt, ...) : type_(type) {
  va_list va;
  va_start(va, fmt);
  err_msg_ = StringPrintfV(fmt, va);
  va_end(va);
}

/* Status::OK = {}; */

// Utilities -------------------------------------------------------------------

const char*
StatusTypeToString(Status::Type type) {
  switch (type) {
    case Status::Type::kOk: return "Ok";
    case Status::Type::kError: return "Error";
    case Status::Type::kDisconnect: return "Disconnect";
  }
  assert(!"Unknown option");
  return "";
}

void
LogStatus(const Status& status) {
  auto msg = StringPrintf("[%s]: %s\n", StatusTypeToString(status.type()),
                          status.err_msg().c_str());
  StdoutAndFlush(msg);
}

}  // namespace warhol
