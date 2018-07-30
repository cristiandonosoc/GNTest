// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <stdarg.h>

#include <string>

namespace warhol {

std::string StringPrintf(const char* fmt, ...);

std::string StringPrintfV(const char* fmt, va_list);

}  // namespace warhol
