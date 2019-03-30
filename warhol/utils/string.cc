// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/utils/string.h"

#include <third_party/stb/stb_sprintf.h>

#include "warhol/utils/log.h"

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

// Trim ------------------------------------------------------------------------

std::string_view Trim(const std::string_view& input,
                      const std::string_view& chars_to_trim) {
  size_t start = input.find_first_not_of(chars_to_trim);
  if (start == std::string_view::npos)
    return {};

  size_t end = input.find_last_not_of(chars_to_trim);
  if (end == std::string_view::npos)
    return input.substr(start);
  return input.substr(start, end - start + 1);
}

// SplitToLines ----------------------------------------------------------------

std::vector<std::string_view> SplitToLines(const std::string_view& input,
                                           const std::string_view& delimiters,
                                           const std::string_view& chars_to_trim) {
  if (input.empty())
    return {};

  std::vector<std::string_view> output;
  size_t current = 0;
  while (current != std::string::npos) {
    size_t end = input.find_first_of(delimiters, current);

    std::string_view view;
    if (end == std::string::npos) {
      view = input.substr(current);
      current = std::string::npos;
    } else {
      view = input.substr(current, end - current);
      current = end + 1;
    }

    view = Trim(view, chars_to_trim);
    if (!view.empty())
      output.push_back(std::move(view));
  }

  return output;
}

}  // namespace warhol
