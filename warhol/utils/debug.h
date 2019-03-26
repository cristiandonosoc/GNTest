// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

namespace warhol {

void HexDump(const uint8_t* data, int lenght);

// Inserts a software breakpoint.
inline void SetBreakpoint() {
#if defined(__x86_64__) || defined(_M_AMD64)
  #if defined(__GNUC__) || defined(__clang__)
    __asm__("int $3");  // x64 software breakpoint.
  #elif defined(_MSC_VER)
    __debugbreak();
  #else
  #error Unsupported compiler.
  #endif
#else
#error Unsupported arch.
#endif
}

}  // namespace warhol
