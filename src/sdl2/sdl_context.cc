// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "sdl_context.h"

#include <assert.h>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

namespace warhol {

struct SDLContextImpl {
  SDL_Window* window;
  SDL_GLContext gl_context;
  int width;
  int height;
};

SDLContext::SDLContext() = default;

SDLContext::~SDLContext() {
  Clear();
}

Status
SDLContext::Init() {
  assert(impl_ == nullptr);
  impl_ = std::make_unique<SDLContextImpl>();

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    return Status("Error loading SDL: %s\n", SDL_GetError());

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  impl_->window = SDL_CreateWindow("Warhol",
                            SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED,
                            1280,
                            720,
                            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  if (!impl_->window)
    return Status("Error creating window: %s", SDL_GetError());

  // Setup an OpenGL context.
  impl_->gl_context = SDL_GL_CreateContext(impl_->window);
  if (!impl_->gl_context)
    return Status("Error creating OpenGL context: %s", SDL_GetError());

  SDL_GL_SetSwapInterval(1);  // Enable v-sync.

  SDL_GetWindowSize(impl_->window, &impl_->width, &impl_->height);

  return Status::Ok();
}

float SDLContext::GetSeconds() const {
  return (float)SDL_GetTicks() / 1000.f;
}

int SDLContext::width() const {
  assert(impl_ != nullptr);
  return impl_->width;
}

int SDLContext::height() const {
  assert(impl_ != nullptr);
  return impl_->height;
}

void
SDLContext::Clear() {
  if (!impl_)
    return;

  if (impl_->gl_context)
    SDL_GL_DeleteContext(impl_->gl_context);
  if (impl_->window)
    SDL_DestroyWindow(impl_->window);
}

SDL_Window* SDLContext::GetWindow() const  {
  if (!impl_)
    return nullptr;
  return impl_->window;
}

}  // namespace warhol
