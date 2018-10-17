// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "sdl_context.h"

namespace warhol {

SDLContext::SDLContext() = default;

SDLContext::~SDLContext() {
  Clear();
}

Status
SDLContext::Init() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    return Status("Error loading SDL: %s\n", SDL_GetError());

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  window = SDL_CreateWindow("Warhol",
                            SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED,
                            1280,
                            720,
                            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  if (!window)
    return Status("Error creating window: %s", SDL_GetError());

  // Setup an OpenGL context.
  gl_context = SDL_GL_CreateContext(window);
  if (!gl_context)
    return Status("Error creating OpenGL context: %s", SDL_GetError());

  SDL_GL_SetSwapInterval(1);  // Enable v-sync.

  return Status::Ok();
}

float SDLContext::GetSeconds() const {
  return (float)SDL_GetTicks() / 1000.f;
}

void
SDLContext::Clear() {
  if (gl_context)
    SDL_GL_DeleteContext(gl_context);
  if (window)
    SDL_DestroyWindow(window);
}

}  // namespace warhol
