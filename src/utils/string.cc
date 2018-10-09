// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <third_party/stb/stb_sprintf.h>

#include "string.h"
#include "log.h"

namespace warhol {

#define STRING_PRINTF_BUF_LEN 256

std::string StringPrintf(const char* fmt, ...) {
  va_list va;
  va_start(va, fmt);
  auto res = StringPrintfV(fmt, va);
  va_end(va);
  return res;
}

std::string StringPrintfV(const char* fmt, va_list va) {
  char buf[STRING_PRINTF_BUF_LEN];
  stbsp_vsnprintf(buf, sizeof(buf), fmt, va);
  return std::string(buf);
}

std::string Concatenate(std::initializer_list<std::string> strings) {
  std::string result;
  size_t size = 0;
  for (const std::string& str : strings)
    size += str.size();

  result.reserve(size);
  for (const std::string& str : strings) {
    LOG(DEBUG) << "Concatenating: " << str;
    result.append(str.data(), str.size());

  }

  return result;
}

}  // namespace warhol
