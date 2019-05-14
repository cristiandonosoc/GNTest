// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <utility>

#include "warhol/utils/macros.h"

namespace warhol {

// Runs a callback when the variable goes out of scope.
template <typename CallbackType>
struct ScopeTrigger {
  ScopeTrigger(CallbackType callback) : callback(std::move(callback)) {}
  ~ScopeTrigger() {
    if (callback)
      callback();
  }

  DELETE_COPY_AND_ASSIGN(ScopeTrigger);
  DELETE_MOVE_AND_ASSIGN(ScopeTrigger);

  CallbackType callback;
};

}  // namespace warhol
