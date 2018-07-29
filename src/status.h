// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <string>

namespace sock {

#define STATUS_MSG_BUF_LEN 256

class Status {
 public:
  enum class Type {
    kOk,
    kError,
  };

  Status();
  Status(std::string err_msg);          // Will set error status.
  Status(Type, std::string err_msg);

  Status(const char* fmt, ...);         // Will set error status.
  Status(Type, const char* fmt, ...);

  bool ok() const { return type_ == Type::kOk; }

  const std::string& err_msg() const { return err_msg_; }
  Type type() const { return type_; }

 private:
  Type type_ = Type::kOk;
  std::string err_msg_;
};



}  // namespace sock
