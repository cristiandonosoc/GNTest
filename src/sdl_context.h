// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "utils/macros.h"
#include "utils/status.h"

namespace warhol {

struct SDLContext {
  SDLContext();
  ~SDLContext();

  Status Init();
  void Clear();

  SDL_Window* window = nullptr;
  SDL_GLContext gl_context = nullptr;

  DELETE_COPY_AND_ASSIGN(SDLContext);
};

}  // namespace warhol
