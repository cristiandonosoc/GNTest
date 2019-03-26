// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "warhol/utils/macros.h"
#include "warhol/window/sdl/def.h"
#include "warhol/window/common/window_backend.h"

namespace warhol {
namespace sdl_opengl {

struct SDLOpenGLWindow : public WindowBackend {
  SDLOpenGLWindow() = default;
  ~SDLOpenGLWindow();  // RAII "semantics".
  DELETE_COPY_AND_ASSIGN(SDLOpenGLWindow);
  DEFAULT_MOVE_AND_ASSIGN(SDLOpenGLWindow);

  bool loaded = false;

  // Virtual Interface ---------------------------------------------------------

  bool Init(Window*) override;
  void Shutdown() override;
  std::pair<WindowEvent*, size_t> UpdateWindow(Window*, InputState*) override;
};

}  // namespace sdl_opengl
}  // namespace warhol
