// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/status.h"

namespace sock {

Status::Status() = default;
Status::Status(std::string err_msg)
    : type_(Type::kError), err_msg_(std::move(err_msg)) {}
Status::Status(Type type, std::string err_msg)
    : type_(type), err_msg_(std::move(err_msg)) {}

}  // namespace sock
