// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/window/sdl/sdl_opengl.h"

#include <memory>

namespace warhol {
namespace sdl_opengl {

// Backend Suscription ---------------------------------------------------------

namespace {

std::unique_ptr<WindowManagerBackend> CreateWindowManager() {
  return std::make_unique<SDLOpenGLWindowManager>();
}

struct BackendSuscriptor {
  BackendSuscriptor() {
    SuscribeWindowManagerBackendFactory(WindowBackendType::kSDLVulkan,
                                        CreateSDLVulkanWindowManager);
  }
};

// Trigger the suscription.
BackendSuscriptor backend_suscriptor;

} // namespace



}  // namespace sdl_opengl
}  // namespace warhol
