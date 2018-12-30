// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/sdl2/sdl_context.h"

#include <limits>

#include "warhol/sdl2/input.h"
#include "warhol/sdl2/def.h"
#include "warhol/utils/assert.h"
#include "warhol/utils/log.h"

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
  float seconds;
  float frame_delta;
  // The accumulated average.
  float frame_delta_accum;
  float frame_delta_average;
  float framerate;

  // TODO(Cristian): When we're interested, start tracking these times.
  float frame_times[kFrameTimesCounts];
  int frame_times_index = 0;
};

SDLContext::SDLContext() {
  utf8_chars_inputted_.reserve(32);
}

SDLContext::~SDLContext() {
  Clear();
}

bool SDLContext::InitOpenGL(uint32_t flags) {
  ASSERT(impl_ == nullptr);
  impl_ = std::make_unique<SDLContextImpl>();

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
    LOG(ERROR) << "Error loading SDL: " << SDL_GetError();
    return false;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  impl_->window = SDL_CreateWindow("Warhol",
                                   SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED,
                                   1280,
                                   720,
                                   SDL_WINDOW_OPENGL | flags);
  if (!impl_->window) {
    LOG(ERROR) << "Error creating window: " << SDL_GetError();
    return false;
  }
  SDL_GetWindowSize(impl_->window, &impl_->width, &impl_->height);

  // Setup an OpenGL context.
  impl_->gl_context = SDL_GL_CreateContext(impl_->window);
  if (!impl_->gl_context) {
    LOG(ERROR) << "Error creating OpenGL context: " << SDL_GetError();
    return false;
  }

  return true;
}

bool SDLContext::InitVulkan(uint32_t flags) {
  ASSERT(impl_ == nullptr);
  impl_ = std::make_unique<SDLContextImpl>();

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
    LOG(ERROR) << "Error loading SDL: " << SDL_GetError();
    return false;
  }

  impl_->window = SDL_CreateWindow("Warhol",
                                   SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED,
                                   1280,
                                   720,
                                   SDL_WINDOW_VULKAN | flags);
  if (!impl_->window) {
    LOG(ERROR) << "Error creating window: " << SDL_GetError();
    return false;
  }
  SDL_GetWindowSize(impl_->window, &impl_->width, &impl_->height);

  return true;
}

int SDLContext::width() const {
  assert(impl_ != nullptr);
  return impl_->width;
}

int SDLContext::height() const {
  assert(impl_ != nullptr);
  return impl_->height;
}

float SDLContext::frame_delta() const { return impl_->frame_delta; }
float SDLContext::frame_delta_average() const {
  return impl_->frame_delta_average;
}
float SDLContext::framerate() const { return impl_->framerate; }

float SDLContext::seconds() const { return impl_->seconds; }

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

  utf8_chars_inputted_.clear();

  // Handle events.
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT: return SDLContext::EventAction::kQuit;
      case SDL_KEYUP: HandleKeyUpEvent(event.key, input); break;
      case SDL_WINDOWEVENT: HandleWindowEvent(event.window); break;
      case SDL_MOUSEWHEEL: HandleMouseWheelEvent(event.wheel, input); break;
      case SDL_TEXTINPUT: {
        // event.text.text is a char[32].
        for (char c : event.text.text) {
          utf8_chars_inputted_.emplace_back(c);
          if (c == 0)
            break;
        }
      }
      default: break;
    }
  }

  HandleKeysDown(input);
  HandleMouse(input);
  return SDLContext::EventAction::kContinue;
}


// Sigh...
#ifdef max
#undef max
#endif

void SDLContext::CalculateFramerate() {
  static uint64_t initial_time = SDL_GetPerformanceCounter();

  // Get the current time.
  uint64_t frequency = SDL_GetPerformanceFrequency();
  uint64_t current_time = SDL_GetPerformanceCounter() - initial_time;

  auto total_time = impl_->total_time;
  impl_->frame_delta =
      (float)(total_time > 0 ? ((double)(current_time - total_time) / frequency)
                             : (1.0 / 60.0));

  impl_->total_time = current_time;
  impl_->seconds = (float)((float)impl_->total_time / (float)frequency);

  // Calculate the rolling average.
  impl_->frame_delta_accum += impl_->frame_delta - impl_->frame_times[impl_->frame_times_index];
  impl_->frame_times[impl_->frame_times_index] = impl_->frame_delta;
  impl_->frame_times_index = (impl_->frame_times_index + 1) % SDLContextImpl::kFrameTimesCounts;
  if (impl_->frame_delta_accum > 0.0) {
    impl_->frame_delta_average = impl_->frame_delta_accum /
                                 SDLContextImpl::kFrameTimesCounts;
  } else {
    impl_->frame_delta_average = std::numeric_limits<float>::max();
  }
  impl_->framerate = 1.0f / impl_->frame_delta_average;
}

void
SDLContext::HandleWindowEvent(const SDL_WindowEvent& window_event) {
  // Fow now we're interested in window changed.
  if (window_event.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
    impl_->width = window_event.data1;
    impl_->height = window_event.data2;

    // Update viewport
    // TODO(Cristian): I shouldn't tie SDL and OpenGL like this.
    /* glViewport(0, 0, width(), height()); */
  }
}


}  // namespace warhol
