// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>

#include "macros.h"

namespace warhol {

class Status {
 public:
  enum class Type {
    kOk,
    kError,
    kDisconnect,    // Server disconnect.
  };

  static Status Ok() { return Status(); }

  Status();
  Status(std::string err_msg);          // Will set error status.
  Status(Type, std::string err_msg);

  Status(const char* fmt, ...) PRINTF_FORMAT(2, 3);   // Will set error status.
  Status(Type, const char* fmt, ...) PRINTF_FORMAT(3, 4);

  bool ok() const { return type_ == Type::kOk; }

  const std::string& err_msg() const { return err_msg_; }
  Type type() const { return type_; }

 private:
  Type type_ = Type::kOk;
  std::string err_msg_;
};

// Utilities -------------------------------------------------------------------

const char* StatusTypeToString();

void LogStatus(const Status&);

}  // namespace warhol
