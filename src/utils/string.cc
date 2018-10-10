// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <third_party/stb/stb_sprintf.h>

#include "src/utils/string.h"

namespace warhol {

std::string StringPrintf(const char* fmt, ...) {
  va_list va;
  va_start(va, fmt);
  auto res = StringPrintfV(fmt, va);
  va_end(va);
  return res;
}

std::string StringPrintfV(const char* fmt, va_list va) {
  char buf[256];
  stbsp_vsnprintf(buf, sizeof(buf), fmt, va);
  return std::string(buf);
}

std::string Concatenate(std::vector<std::string> strings) {
  std::string result;
  size_t size = 0;
  for (const std::string& str : strings)
    size += str.size();

  result.reserve(size);
  for (std::string& str : strings) {
    result.append(std::move(str));
  }

  return result;
}

}  // namespace warhol
