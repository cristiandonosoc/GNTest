// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "utils/macros.h"
#include "utils/status.h"

namespace warhol {

class SDLContext {
 public:
  SDLContext();
  ~SDLContext();

  Status Init();
  void Clear();

  SDL_Window* window = nullptr;
  SDL_GLContext gl_context = nullptr;

  // Returns the seconds since Init() was called. This is a fractional number.
  float GetSeconds() const;

  DELETE_COPY_AND_ASSIGN(SDLContext);
};

}  // namespace warhol
