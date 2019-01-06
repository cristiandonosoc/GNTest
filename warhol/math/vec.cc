// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/math/vec.h"

namespace warhol {

template <>
std::string Pair<int>::ToString() const {
  return StringPrintf("X: %d, Y: %d", x, y);
}

template <>
std::string Pair<uint32_t>::ToString() const {
  return StringPrintf("X: %d, Y: %d", x, y);
}

template <>
std::string Pair<float>::ToString() const {
  return StringPrintf("X: %f, Y: %f", (float)x, (float)y);
}

// Generate the functions explicitly, as MSVC won't generate templates it does
// not explicitly need.
#if defined(_MSC_VER)
template <typename T>
static std::string GenTemplate() {
  static Pair<T> a;
  return a.ToString();
};
static auto call_float_t = GenTemplate<float>();
static auto call_int_t = GenTemplate<int>();
static auto call_uint32_t = GenTemplate<uint32_t>();
#endif

}  // namespace warhol
