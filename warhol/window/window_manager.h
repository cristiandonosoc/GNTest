// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdlib.h>

#include <utility>
#include <vector>

#include "warhol/utils/macros.h"

namespace warhol {

struct WindowManagerBackend;

struct WindowManager {
  static constexpr size_t kRollingAverageFrames = 60;

  bool valid() const { return backend.valid(); }
  WindowManagerBackend::Type backend_type() const { return backend.type; }
  WindowManagerBackend::Interface& interface() { return backend.interface; }

  WindowManager();
  ~WindowManager();
  DELETE_COPY_AND_ASSIGN(WindowManager);
  DELETE_MOVE_AND_ASSIGN(WindowManager);

  size_t width = 0;
  size_t height = 0;
  float frame_delta = 0;          // Delta within the last frame in seconds.
  float frame_delta_average = 0;  // Rolling average over kRollingAverageFrames.
  float frame_rate = 0;           // 1 / frame_delta_average.
  float seconds = 0;              // Seconds since Init was called.

  std::unique_ptr<WindowManagerBackend> backend;
};

// WindowManager must be already set with |type|.
bool InitWindowManager(WindowManager*, WindowManagerBackend::Type,
                       uint64_t flags);
std::pair<WindowEvent*, size_t> NewFrame(WindowManager*, InputState*);
void ShutdownWindowManager(WindowManager*);

}  // namespace warhol
