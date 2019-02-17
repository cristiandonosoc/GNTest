// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/window/window_manager.h"

#include "warhol/utils/assert.h"
#include "warhol/utils/log.h"

namespace warhol {

// WindowManager ---------------------------------------------------------------

WindowManager::WindowManager() = default;
WindowManager::~WindowManager() {
  if (valid())
    ShutdownWindowManager(this);
}


bool InitWindowManager(WindowManager* window,
                       WindowManagerBackend::Type type,
                       uint64_t flags) {
  if (type == WindowManagerBackend::Type::kLast) {
    LOG(ERROR) << "Unset WindowManager type.";
    return false;
  }

  window->backend = GetWindowManagerBackend(type);
  window->backend.window_manager = window;  // Set the backpointer.

  // Initialize the backend.
  auto& interface = window->backend.interface;
  return interface.Init(&window->backend, flags);
}

std::pair<WindowEvent*, size_t>
NewFrame(WindowManager* window, InputState* input) {
  ASSERT(window->valid());
  auto& interface = window->interface();
  return interface.NewFrame(window, input);
}

void ShutdownWindowManager(WindowManager* window) {
  ASSERT(window->valid());
  auto& interface = window->interface();
  interface.Shutdown(&window->backend);
}

}  // namespace warhol
