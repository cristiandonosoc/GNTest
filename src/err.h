// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <string>

namespace sock {

class Err {
 public:
  enum class Status {
    kOk,
    kError,
  };

  Err();
  Err(std::string err_msg);   // Will set error status.
  Err(Status, std::string);

 private:
  Status status_ = Status::kOk;
  std::string err_msg_;
};


}  // namespace sock
