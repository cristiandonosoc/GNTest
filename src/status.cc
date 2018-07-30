// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <assert.h>

#include "status.h"
#include "string.h"

namespace warhol {

Status::Status() = default;

Status::Status(std::string err_msg)
    : type_(Type::kError), err_msg_(std::move(err_msg)) {}
Status::Status(Type type, std::string err_msg)
    : type_(type), err_msg_(std::move(err_msg)) {}

Status::Status(const char *fmt, ...) : type_(Type::kError) {
  va_list va;
  va_start(va, fmt);
  err_msg_ = StringPrintfV(fmt, va);
  va_end(va);
}

Status::Status(Type type, const char *fmt, ...) : type_(type) {
  va_list va;
  va_start(va, fmt);
  err_msg_ = StringPrintfV(fmt, va);
  va_end(va);
}

// Utilities -------------------------------------------------------------------

namespace {

const std::string kEmpty;

}  // namespace


const std::string& StatusTypeToString(Status::Type type) {
  switch (type) {
    case Status::Type::kOk: {
      static std::string kOk = "Ok";
      return kOk;
    }
    case Status::Type::kError: {
      static std::string kError = "Error";
      return kError;
    }
    case Status::Type::kDisconnect: {
      static std::string kError = "Disconenct";
      return kError;
    }
  }
  assert(!"Error");
  return kEmpty;
}

void LogStatus(const Status &status) {
  fprintf(stderr, "[%s]: %s\n", StatusTypeToString(status.type()).c_str(),
                                status.err_msg().c_str());
}

}  // namespace warhol
