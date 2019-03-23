// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <string>

namespace warhol {

constexpr size_t kKilobyte = 1024;
constexpr size_t kMegabyte = 1024 * 1024;
constexpr size_t kGigabyte = 1024 * 1024 * 1024;

#define KILOBYTES(count) (count * kKilobyte)
#define MEGABYTES(count) (count * kMegabyte)
#define GIGABYTES(count) (count * kGigabyte)

inline float ToKilobytes(size_t bytes) {
  return (float)bytes / kKilobyte;
}
inline float ToMegabytes(size_t bytes) {
  return (float)bytes / kMegabyte;
}
inline float ToGigabytes(size_t bytes) {
  return (float)bytes / kGigabyte;
}

std::string BytesToString(size_t bytes);

}  // namespace warhol
