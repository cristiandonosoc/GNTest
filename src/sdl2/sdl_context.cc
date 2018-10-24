// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "sdl_context.h"

#include <assert.h>

#include <limits>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "src/sdl2/input.h"

namespace warhol {

struct SDLContextImpl {
  // Amount of frames to keep track of in order to get an average of the frame
  // times.
  static constexpr int kFrameTimesCounts = 128;

  SDL_Window* window;
  SDL_GLContext gl_context;
  int width;
  int height;

  // Total time since the start of the game.
  uint64_t total_time = 0;
  double frame_delta;
  // The accumulated average.
  double frame_delta_accum;
  double frame_delta_average;
  double framerate;

  // TODO(Cristian): When we're interested, start tracking these times.
  double  frame_times[kFrameTimesCounts];
  int frame_times_index = 0;
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

double
SDLContext::frame_delta() const { return impl_->frame_delta; }
double SDLContext::frame_delta_average() const {
  return impl_->frame_delta_average;
}
double SDLContext::framerate() const { return impl_->framerate; }
double SDLContext::frame_delta_accum() const { return impl_->frame_delta_accum; }

void
SDLContext::Clear() {
  if (!impl_)
    return;

  if (impl_->gl_context)
    SDL_GL_DeleteContext(impl_->gl_context);
  if (impl_->window)
    SDL_DestroyWindow(impl_->window);
}

SDL_Window* SDLContext::get_window() const  {
  if (!impl_)
    return nullptr;
  return impl_->window;
}

SDLContext::EventAction
SDLContext::NewFrame(InputState* input) {
  CalculateFramerate();

  // We do the frame flip.
  InputState::InitFrame(input);

  // Handle events.
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT: return SDLContext::EventAction::kQuit;
      case SDL_KEYUP: HandleKeyUp(event.key, input); break;
      case SDL_WINDOWEVENT:
        LOG(DEBUG) << "Window event!";
        break;
      default: break;
    }
  }

  HandleKeysDown(input);
  HandleMouse(input);
  return SDLContext::EventAction::kContinue;
}

void SDLContext::CalculateFramerate() {
  // Get the current time.
  uint64_t frequency = SDL_GetPerformanceFrequency();
  uint64_t current_time = SDL_GetPerformanceCounter();

  auto total_time = impl_->total_time;
  impl_->frame_delta = total_time > 0
                           ? ((double)(current_time - total_time) / frequency)
                           : (1.0 / 60.0);
  impl_->total_time = current_time;

  // Calculate the rolling average.
  impl_->frame_delta_accum += impl_->frame_delta - impl_->frame_times[impl_->frame_times_index];
  impl_->frame_times[impl_->frame_times_index] = impl_->frame_delta;
  impl_->frame_times_index = (impl_->frame_times_index + 1) % SDLContextImpl::kFrameTimesCounts;
  if (impl_->frame_delta_accum > 0.0) {
    impl_->frame_delta_average = impl_->frame_delta_accum /
                                 SDLContextImpl::kFrameTimesCounts;
  } else {
    impl_->frame_delta_average = std::numeric_limits<double>::max();
  }
  impl_->framerate = 1.0 / impl_->frame_delta_average;
}


}  // namespace warhol
