// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/window/sdl/sdl_opengl.h"
#include "warhol/window/common/window.h"

#include <memory>

namespace warhol {
namespace sdl_opengl {

// Backend Suscription ---------------------------------------------------------

namespace {

std::unique_ptr<WindowBackend> CreateWindow() {
  return std::make_unique<SDLOpenGLWindow>();
}

struct BackendSuscriptor {
  BackendSuscriptor() {
    SuscribeWindowBackendFactoryFunction(WindowBackendType::kSDLVulkan,
                                         CreateWindow);
  }
};

// Trigger the suscription.
BackendSuscriptor backend_suscriptor;

} // namespace

}  // namespace sdl_opengl
}  // namespace warhol
