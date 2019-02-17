// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

namespace warhol {

void HexDump(const uint8_t* data, int lenght);

// Inserts a software breakpoint.
inline void SetBreakpoint() {
#ifdef __x86_64__
  __asm__("int $3");  // x64 software breakpoint.
#else
#error Unsupported arch.
#endif
}

}  // namespace warhol
