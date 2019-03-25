// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/window/common/window_backend.h"

#include "warhol/utils/assert.h"

namespace warhol {

std::vector<const char*>
WindowBackend::GetVulkanInstanceExtensions() {
  NOT_REACHED("This function must be subclassed.");
  return {};
}

bool WindowBackend::CreateVulkanSurface(void*, void*) {
  NOT_REACHED("This function must be subclassed.");
  return false;
}

}  // namespace warhol
