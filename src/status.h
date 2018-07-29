// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <string>

namespace sock {

class Status {
 public:
  enum class Type {
    kOk,
    kError,
  };

  Status();
  Status(std::string err_msg);   // Will set error status.
  Status(Type, std::string err_msg);

 private:
  Type type_ = Type::kOk;
  std::string err_msg_;
};



}  // namespace sock
