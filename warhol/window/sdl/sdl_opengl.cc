// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/window/sdl/sdl_opengl.h"

#include <memory>

#include "warhol/input/input.h"
#include "warhol/utils/log.h"
#include "warhol/window/common/window.h"
#include "warhol/window/sdl/sdl_input.h"

namespace warhol {
namespace sdl {

// Backend Suscription ---------------------------------------------------------

namespace {

std::unique_ptr<WindowBackend> CreateWindow() {
  return std::make_unique<SDLOpenGLWindow>();
}

struct BackendSuscriptor {
  BackendSuscriptor() {
    SuscribeWindowBackendFactoryFunction(WindowBackendType::kSDLOpenGL,
                                         CreateWindow);
  }
};

// Trigger the suscription.
BackendSuscriptor backend_suscriptor;

} // namespace

// Shutdown --------------------------------------------------------------------

namespace {

void SDLOpenGLShutdown(SDLOpenGLWindow* sdl) {
  if (sdl->gl_context.has_value()) {
    SDL_GL_DeleteContext(sdl->gl_context.value);
    sdl->gl_context.clear();
  }

  if (sdl->sdl_window.has_value()) {
    SDL_DestroyWindow(sdl->sdl_window.value);
    sdl->sdl_window.clear();
  }

  if (Valid(&sdl->memory_pool))
    ShutdownMemoryPool(&sdl->memory_pool);
}

}  // namespace

void SDLOpenGLWindow::Shutdown() {
  SDLOpenGLShutdown(this);
}

// Init ------------------------------------------------------------------------

namespace {

bool SDLOpenGLInit(SDLOpenGLWindow* sdl, Window* window) {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    LOG(ERROR) << "Error loading SDL: " << SDL_GetError();
    return false;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  sdl->sdl_window = SDL_CreateWindow("Warhol",
                                     SDL_WINDOWPOS_CENTERED,
                                     SDL_WINDOWPOS_CENTERED,
                                     1280, 720,
                                     SDL_WINDOW_OPENGL);
  if (!sdl->sdl_window.has_value()) {
    LOG(ERROR) << "Error creating window: " << SDL_GetError();
    SDLOpenGLShutdown(sdl);
    return false;
  }

  // Setup the OpenGL Context.
  sdl->gl_context = SDL_GL_CreateContext(sdl->sdl_window.value);
  if (!sdl->gl_context.has_value()) {
    LOG(ERROR) << "Error creating OpenGL context: " << SDL_GetError();
    SDLOpenGLShutdown(sdl);
    return false;
  }


  SDL_GL_SetSwapInterval(1);  // Enable v-sync.
  SDL_GetWindowSize(sdl->sdl_window.value, &window->width, &window->height);

  InitMemoryPool(&sdl->memory_pool, KILOBYTES(1));
  return true;
}

}  // namespace

bool SDLOpenGLWindow::Init(Window* window) {
  return SDLOpenGLInit(this, window);
}

// UpdateWindow ----------------------------------------------------------------

namespace {

// TODO: Move this to platform!!!!
void CalculateFramerate(Window* window) {
  static uint64_t initial_time = SDL_GetPerformanceCounter();

  // Get the current time.
  uint64_t frequency = SDL_GetPerformanceFrequency();
  uint64_t current_time = SDL_GetPerformanceCounter() - initial_time;

  auto total_time = window->total_time;
  window->frame_delta =
      (float)(total_time > 0 ? ((double)(current_time - total_time) / frequency)
                             : (1.0 / 60.0));

  window->total_time = current_time;
  window->seconds = (float)((float)window->total_time / (float)frequency);

  // Calculate the rolling average.
  window->frame_delta_accum +=
      window->frame_delta - window->frame_times[window->frame_times_index];
  window->frame_times[window->frame_times_index] = window->frame_delta;
  window->frame_times_index =
      (window->frame_times_index + 1) % Window::kFrameTimesCounts;
  if (window->frame_delta_accum > 0.0) {
    window->frame_delta_average =
        window->frame_delta_accum / Window::kFrameTimesCounts;
  } else {
    window->frame_delta_average = std::numeric_limits<float>::max();
  }
  window->frame_rate = 1.0f / window->frame_delta_average;
}

void PushEvent(SDLOpenGLWindow* sdl, WindowEvent event) {
  ASSERT(sdl->event_index < ARRAY_SIZE(sdl->events));
  sdl->events[sdl->event_index++] = event;
}

void PushUtf8Char(SDLOpenGLWindow* sdl, char c) {
  ASSERT(sdl->utf8_index < ARRAY_SIZE(sdl->utf8_chars_inputted));
  sdl->utf8_chars_inputted[sdl->utf8_index++] = c;
}

void HandleWindowEvent(const SDL_WindowEvent& window_event,
                       SDLOpenGLWindow* sdl,
                       Window* window) {
  // Fow now we're interested in window changed.
  if (window_event.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
    PushEvent(sdl, WindowEvent::kWindowResize);
    window->width = window_event.data1;
    window->height = window_event.data2;
  }
}

LinkedList<WindowEvent>
SDLOpenGLUpdateWindow(SDLOpenGLWindow* sdl, Window* window, InputState* input) {
  ASSERT(Valid(sdl));

  // Restart the state.
  sdl->event_index = 0;
  sdl->utf8_index = 0;
  ResetMemoryPool(&sdl->memory_pool);

  CalculateFramerate(window);
  InputState::InitFrame(input);  // We do the frame flip.

  // Handle events.
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT: PushEvent(sdl, WindowEvent::kQuit); break;
      case SDL_KEYUP: HandleKeyUpEvent(event.key, input); break;
      case SDL_MOUSEWHEEL: HandleMouseWheelEvent(event.wheel, input); break;
      case SDL_WINDOWEVENT: HandleWindowEvent(event.window, sdl, window); break;
      case SDL_TEXTINPUT: {
        // event.text.text is a char[32].
        for (char c : event.text.text) {
          PushUtf8Char(sdl, c);
          if (c == 0)
            break;
        }
      }
      default: break;
    }
  }

  HandleKeysDown(input);
  HandleMouse(input);

  // Chain the events into a linked list.
  LinkedList<WindowEvent> event_list;
  for (int i = 0; i < sdl->event_index; i++) {
    WindowEvent* e = PushIntoListFromMemoryPool(&event_list, &sdl->memory_pool);
    *e = sdl->events[i];
  }
  return event_list;
}

}  // namespace

LinkedList<WindowEvent>
SDLOpenGLWindow::UpdateWindow(Window* window, InputState* input) {
  return SDLOpenGLUpdateWindow(this, window, input);
}

// SwapBuffers -----------------------------------------------------------------

void SDLOpenGLWindow::SwapBuffers() {
  SDL_GL_SwapWindow(this->sdl_window.value);
}

// Misc ------------------------------------------------------------------------

SDLOpenGLWindow::~SDLOpenGLWindow() {
  if (Valid(this))
    SDLOpenGLShutdown(this);
}

}  // namespace sdl
}  // namespace warhol
