// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.


#include <third_party/stb/stb_sprintf.h>

#include "src/status.h"

namespace sock {

Status::Status() = default;

Status::Status(std::string err_msg)
    : type_(Type::kError), err_msg_(std::move(err_msg)) {}
Status::Status(Type type, std::string err_msg)
    : type_(type), err_msg_(std::move(err_msg)) {}

Status::Status(const char *fmt, ...) : type_(Type::kError) {
  char buf[STATUS_MSG_BUF_LEN];

  va_list va;
  va_start(va, fmt);
  stbsp_vsnprintf(buf, sizeof(buf), fmt, va);
  va_end(va);
  err_msg_ = buf;
}

Status::Status(Type type, const char *fmt, ...) : type_(type) {
  char buf[STATUS_MSG_BUF_LEN];

  va_list va;
  va_start(va, fmt);
  stbsp_vsnprintf(buf, sizeof(buf), fmt, va);
  va_end(va);
  err_msg_ = buf;
}

}  // namespace sock
