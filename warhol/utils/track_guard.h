// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "warhol/utils/assert.h"
#include "warhol/utils/clear_on_move.h"
#include "warhol/utils/macros.h"

namespace warhol {

// A track guard is for protecting when a tracked resource that's being tracked
// by pointer is moved. This is important because track by pointer requires that
// an object remains stable in memory.
//
// If active, the guard will assert if moved.
struct TrackGuard {
  TrackGuard() = default;
  ~TrackGuard() {
    ASSERT(!active.value);
  }
  DELETE_COPY_AND_ASSIGN(TrackGuard);
  DEFAULT_MOVE_AND_ASSIGN(TrackGuard);

  ClearOnMove<bool> active = false;
};

inline bool Active(TrackGuard* guard) { return guard->active.value; }

}  // namespace warhol
