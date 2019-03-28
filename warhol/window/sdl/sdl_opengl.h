// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "warhol/containers/linked_list.h"
#include "warhol/memory/memory_pool.h"
#include "warhol/utils/clear_on_move.h"
#include "warhol/utils/macros.h"
#include "warhol/window/sdl/def.h"
#include "warhol/window/common/window_backend.h"

namespace warhol {
namespace sdl {

struct SDLOpenGLWindow : public WindowBackend {
  SDLOpenGLWindow() = default;
  ~SDLOpenGLWindow();  // RAII "semantics".
  DELETE_COPY_AND_ASSIGN(SDLOpenGLWindow);
  DEFAULT_MOVE_AND_ASSIGN(SDLOpenGLWindow);

  ClearOnMove<SDL_Window*> sdl_window = nullptr;
  ClearOnMove<SDL_GLContext> gl_context = NULL;
  MemoryPool memory_pool;

  // Array of events that can happen within a frame.
  int event_index = 0;
  WindowEvent events[4];

  int utf8_index = 0;
  char utf8_chars_inputted[256];

  // Virtual Interface ---------------------------------------------------------

  bool Init(Window*) override;
  void Shutdown() override;
  LinkedList<WindowEvent> UpdateWindow(Window*, InputState*) override;
  void SwapBuffers() override;
};

inline bool Valid(SDLOpenGLWindow* sdl) {
  return sdl->sdl_window.has_value() && sdl->gl_context.has_value();
}

}  // namespace sdl
}  // namespace warhol
