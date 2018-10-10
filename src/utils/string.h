// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <stdarg.h>

#include <string>
#include <vector>

namespace warhol {

std::string StringPrintf(const char* fmt, ...);

std::string StringPrintfV(const char* fmt, va_list);

std::string Concatenate(std::vector<std::string> strings);

}  // namespace warhol
