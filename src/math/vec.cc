// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/math/vec.h"

namespace warhol {

template <>
std::string Pair<float>::ToString() const {
  return StringPrintf("X: %f, Y: %f", (float)x, (float)y);
}

template <>
std::string Pair<int>::ToString() const {
  return StringPrintf("X: %d, Y: %d", x, y);
}

// Generate the functions explicitly, as MSVC won't generate templates it does
// not explicitly need.
template <typename T>
static std::string GenTemplate() {
  static Pair<T> a;
  return a.ToString();
}
static auto float_t = GenTemplate<float>();
static auto int_t = GenTemplate<int>();


}  // namespace warhol
