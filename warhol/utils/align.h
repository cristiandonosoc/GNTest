// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

namespace warhol {

// This will return the next address from |x| aligned to |align| bits.
inline uint64_t Align(uint64_t x, uint64_t align) {
  uint64_t mask = align - 1;
  return (x + mask) & ~mask;
}

}  // namespace
