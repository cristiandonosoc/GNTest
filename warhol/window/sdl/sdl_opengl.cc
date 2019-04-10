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

  sdl->window = nullptr;
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

  sdl->window = window;
  InitMemoryPool(&sdl->memory_pool, KILOBYTES(1));
  return true;
}

}  // namespace

bool SDLOpenGLWindow::Init(Window* w) {
  return SDLOpenGLInit(this, w);
}

// UpdateWindow ----------------------------------------------------------------

namespace {

void PushEvent(SDLOpenGLWindow* sdl, WindowEvent event) {
  ASSERT(sdl->event_index < ARRAY_SIZE(sdl->events));
  sdl->events[sdl->event_index++] = event;
}

void PushUtf8Char(SDLOpenGLWindow* sdl, char c) {
  Window* window = sdl->window;
  ASSERT(window->utf8_index < ARRAY_SIZE(window->utf8_chars_inputted));
  window->utf8_chars_inputted[window->utf8_index++] = c;
}

void ResetUtf8(Window* window) {
  for (int i = 0; i < ARRAY_SIZE(window->utf8_chars_inputted); i++) {
    window->utf8_chars_inputted[i] = 0;
  }
  window->utf8_index = 0;
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
  ResetUtf8(sdl->window);
  ResetMemoryPool(&sdl->memory_pool);

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
SDLOpenGLWindow::UpdateWindow(Window* w, InputState* input) {
  return SDLOpenGLUpdateWindow(this, w, input);
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
