// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <utility>

#include "warhol/utils/macros.h"

namespace warhol {

// Runs a callback when the variable goes out of scope.
template <typename CALLBACK>
struct ScopeTrigger {
  ScopeTrigger(CALLBACK callback) : callback(std::move(callback)) {}
  ~ScopeTrigger() {
    callback();
  }

  DELETE_COPY_AND_ASSIGN(ScopeTrigger);
  DELETE_MOVE_AND_ASSIGN(ScopeTrigger);

  CALLBACK callback;
};

}  // namespace warhol
