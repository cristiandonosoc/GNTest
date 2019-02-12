// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdlib.h>

#include <utility>
#include <vector>

#include "warhol/utils/macros.h"
#include "warhol/window/common/window_manager_backend.h"

namespace warhol {

struct WindowManager {
  static constexpr size_t kRollingAverageFrames = 60;
  WindowManagerBackend::Interface& interface() { return backend.interface; }

  WindowManagerBackend backend;

  size_t width = 0;
  size_t height = 0;

  float frame_delta = 0;          // Delta within the last frame in seconds.
  float frame_delta_average = 0;  // Rolling average over kRollingAverageFrames.
  float frame_rate = 0;           // 1 / frame_delta_average.
  float seconds = 0;              // Seconds since Init was called.
};

// WindowManager must be already set with |type|.
bool InitWindowManager(WindowManager*, WindowManagerBackend::Type,
                       uint64_t flags);
std::pair<WindowEvent*, size_t> NewFrame(WindowManager*, InputState*);

}  // namespace warhol
