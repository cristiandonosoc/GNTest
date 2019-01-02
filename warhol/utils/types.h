// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <limits>

namespace warhol {

struct Limits {
  static constexpr uint16_t kUint8Min = std::numeric_limits<uint8_t>::min();
  static constexpr uint16_t kUint8Max = std::numeric_limits<uint8_t>::max();

  static constexpr uint16_t kUint16Min = std::numeric_limits<uint16_t>::min();
  static constexpr uint16_t kUint16Max = std::numeric_limits<uint16_t>::max();

  static constexpr uint16_t kUint32Min = std::numeric_limits<uint32_t>::min();
  static constexpr uint32_t kUint32Max = std::numeric_limits<uint32_t>::max();

  static constexpr uint32_t kUint64Min = std::numeric_limits<uint64_t>::min();
  static constexpr uint64_t kUint64Max = std::numeric_limits<uint64_t>::max();
};

}  // namespace warhol
