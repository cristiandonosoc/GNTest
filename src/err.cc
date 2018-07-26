// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/err.h"

namespace sock {

Err::Err() = default;
Err::Err(std::string err_msg) : status_(Status::kError), err_msg_(std::move(err_msg)) {}
Err::Err(Status status, std::string err_msg) : status_(status), err_msg_(std::move(err_msg)) {}

}  // namespace sock
