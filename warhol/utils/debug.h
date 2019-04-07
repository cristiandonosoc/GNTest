// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

namespace warhol {

void HexDump(const uint8_t* data, int lenght);

// Inserts a software breakpoint.
inline void SetBreakpoint() {
#if defined(__GNUC__) || defined(__clang__)
  __builtin_trap();
#elif defined(_MSC_VER)
  __debugbreak();
#else
#error Unsupported compiler.
#endif
}

}  // namespace warhol
