// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/math/vec.h"

namespace warhol {

template <>
std::string
Pair<float>::ToString() const {
  return StringPrintf("X: %f, Y: %f", (float)x, (float)y);
}

}  // namespace warhol
