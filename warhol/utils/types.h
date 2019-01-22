// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <limits>

namespace warhol {

struct MemoryDefs {
  static constexpr size_t kKilobyte = 1024;
  static constexpr size_t kMegabyte = 1024 * 1024;
  static constexpr size_t kGigabyte = 1024 * 1024 * 1024;
};

#define KILOBYTES(count) count * ::warhol::MemoryDefs::kKilobyte
#define MEGABYTES(count) count * ::warhol::MemoryDefs::kMegabyte
#define GIGABYTES(count) count * ::warhol::MemoryDefs::kGigabyte

inline float ToKilobytes(size_t bytes) {
  return (float)bytes / ::warhol::MemoryDefs::kKilobyte;
}
inline float ToMegabytes(size_t bytes) {
  return (float)bytes / ::warhol::MemoryDefs::kMegabyte;
}
inline float ToGigabytes(size_t bytes) {
  return (float)bytes / ::warhol::MemoryDefs::kGigabyte;
}

}  // namespace warhol
