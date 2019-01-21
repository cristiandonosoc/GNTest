// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>

namespace warhol {

// This is the generic swap algorithm. Some specializations are to be done in
// cases such as strings, which will copy unnecesary amount of data.
template <typename T>
inline void SwapValues(T& a, T& b) {
  T tmp = a;
  a = b;
  b = tmp;
}

template <>
inline void SwapValues(std::string& a, std::string& b) {
  a.swap(b);
}

}  // namespace warhol
