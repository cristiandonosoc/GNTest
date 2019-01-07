// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "warhol/utils/macros.h"

namespace warhol {

// A Unique represents a normal integer type that gets clear on move, instead
// of the copying the compiler does.
template <typename T>
struct ClearOnMove{
  T value;

  ClearOnMove() = default;
  ClearOnMove(T value) : value(value) {}
  DELETE_COPY_AND_ASSIGN(ClearOnMove);

  ClearOnMove& operator=(T v) {
    value = v;
    return *this;
  }

  void clear() { value = 0; }

  ClearOnMove(ClearOnMove&& other) {
    value = other.value;
    other.value = 0;
  }
  ClearOnMove& operator=(ClearOnMove&& other) {
    value = other.value;
    other.value = 0;
    return *this;
  }
};

}  // namespace warhol
