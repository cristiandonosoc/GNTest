// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/window/common/window_manager_backend.h"

#include "warhol/utils/assert.h"
#include "warhol/utils/log.h"

namespace warhol {

std::vector<const char*>
WindowManagerBackend::GetVulkanInstanceExtensions() {
  NOT_REACHED("This function must be subclassed.");
  return {};
}


bool WindowManagerBackend::CreateVulkanSurface(void*, void*) {
  NOT_REACHED("This function must be subclassed.");
  return false;
}

const char* WindowEvent::TypeToString(WindowEvent::Type event) {
  switch (event) {
    case WindowEvent::Type::kQuit: return "Quit";
    case WindowEvent::Type::kWindowResize: return "WindowResize";
    case WindowEvent::Type::kLast: return "Last";
  }

  NOT_REACHED("Unknown WindowEvent.");
  return nullptr;
}

}  // namespace warhol
