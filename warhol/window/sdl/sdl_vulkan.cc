// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/window/sdl/sdl_vulkan.h"

#include <SDL2/SDL_vulkan.h>

#include "warhol/input/input.h"
#include "warhol/utils/assert.h"
#include "warhol/utils/log.h"
#include "warhol/window/common/window.h"

// Sigh... Well done windows.
#ifdef max
#undef max
#endif

namespace warhol {
namespace sdl_vulkan {

// Backend Suscription ---------------------------------------------------------

namespace {

std::unique_ptr<WindowBackend> CreateSDLVulkanWindow() {
  return std::make_unique<SDLVulkanWindow>();
}

struct BackendSuscriptor {
  BackendSuscriptor() {
    SuscribeWindowBackendFactoryFunction(WindowBackendType::kSDLVulkan,
                                         CreateSDLVulkanWindow);
  }
};

// Trigger the suscription.
BackendSuscriptor backend_suscriptor;

}  // namespace

// SDLVulkanWindow ------------------------------------------------------

SDLVulkanWindow::~SDLVulkanWindow() {
  if (Valid(this))
    Shutdown();
}

namespace {

// Passes the info from the backend to the unified Window interface.
void PassInfo(SDLVulkanWindow* from, Window* to) {
  to->width = from->width;
  to->height = from->height;

  to->frame_delta = from->frame_delta;
  to->frame_delta_average = from->frame_delta_average;
  to->frame_rate = from->frame_rate;
  to->seconds = from->seconds;
}

// Init ------------------------------------------------------------------------

bool InitSDLVulkan(SDLVulkanWindow* sdl) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
    LOG(ERROR) << "Error loading SDL: " << SDL_GetError();
    return false;
  }

  sdl->window = SDL_CreateWindow("Warhol",
                                 SDL_WINDOWPOS_CENTERED,
                                 SDL_WINDOWPOS_CENTERED,
                                 1280,
                                 720,
                                 SDL_WINDOW_VULKAN);
  if (!sdl->window) {
    LOG(ERROR) << "Error creating window: " << SDL_GetError();
    return false;
  }

  SDL_GetWindowSize(sdl->window, &sdl->width, &sdl->height);
  PassInfo(sdl, sdl->window_manager);
  return true;
}

// Shutdown --------------------------------------------------------------------

void ShutdownSDLVulkan(SDLVulkanWindow* sdl) {
  ASSERT(Valid(sdl));
  if (sdl->window)
    SDL_DestroyWindow(sdl->window);
  return;
}

// NewFrame --------------------------------------------------------------------

void CalculateFramerate(SDLVulkanWindow* sdl);
void HandleKeyUpEvent(const SDL_KeyboardEvent*, InputState*);
void HandleKeysDown(InputState*);
void HandleMouse(InputState*);
void HandleMouseWheelEvent(const SDL_MouseWheelEvent*, InputState*);
void HandleWindowEvent(SDLVulkanWindow*, const SDL_WindowEvent*);

std::pair<WindowEvent*, size_t> NewFrameSDLVulkan(SDLVulkanWindow* sdl,
                                                  Window* window,
                                                  InputState* input) {
  ASSERT(Valid(sdl));

  sdl->events.clear();
  sdl->utf8_chars_inputted.clear();

  CalculateFramerate(sdl);
  InputState::InitFrame(input);  // We do the frame flip.

  // Handle events.
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT: sdl->events.push_back({WindowEvent::kQuit});
      case SDL_KEYUP: HandleKeyUpEvent(&event.key, input); break;
      case SDL_WINDOWEVENT: HandleWindowEvent(sdl, &event.window); break;
      case SDL_MOUSEWHEEL: HandleMouseWheelEvent(&event.wheel, input); break;
      case SDL_TEXTINPUT: {
        // event.text.text is a char[32].
        for (char c : event.text.text) {
          sdl->utf8_chars_inputted.emplace_back(c);
          if (c == 0)
            break;
        }
      }
      default: break;
    }
  }

  HandleKeysDown(input);
  HandleMouse(input);

  PassInfo(sdl, sdl->window_manager);
  return {sdl->events.empty() ? nullptr : sdl->events.data(),
          sdl->events.size()};
}

void CalculateFramerate(SDLVulkanWindow* sdl) {
  static uint64_t initial_time = SDL_GetPerformanceCounter();

  // Get the current time.
  uint64_t frequency = SDL_GetPerformanceFrequency();
  uint64_t current_time = SDL_GetPerformanceCounter() - initial_time;

  auto total_time = sdl->total_time;
  sdl->frame_delta =
      (float)(total_time > 0 ? ((double)(current_time - total_time) / frequency)
                             : (1.0 / 60.0));

  sdl->total_time = current_time;
  sdl->seconds = (float)((float)sdl->total_time / (float)frequency);

  // Calculate the rolling average.
  sdl->frame_delta_accum +=
      sdl->frame_delta - sdl->frame_times[sdl->frame_times_index];
  sdl->frame_times[sdl->frame_times_index] = sdl->frame_delta;
  sdl->frame_times_index =
      (sdl->frame_times_index + 1) % SDLVulkanWindow::kFrameTimesCounts;
  if (sdl->frame_delta_accum > 0.0) {
    sdl->frame_delta_average =
        sdl->frame_delta_accum / SDLVulkanWindow::kFrameTimesCounts;
  } else {
    sdl->frame_delta_average = std::numeric_limits<float>::max();
  }
  sdl->frame_rate = 1.0f / sdl->frame_delta_average;
}

void HandleKeyUpEvent(const SDL_KeyboardEvent* key_event, InputState* input) {
  switch (key_event->keysym.scancode) {
    case SDL_SCANCODE_ESCAPE: input->keys_up[GET_KEY(Escape)] = true; break;
    default: break;
  }
}

#define SET_SDL_KEY(sdl_key, key)        \
  if (key_state[SDL_SCANCODE_##sdl_key]) \
    input->keys_down[GET_KEY(key)] = true;

void HandleKeysDown(InputState* input) {
  const uint8_t* key_state = SDL_GetKeyboardState(0);
  SET_SDL_KEY(UP, Up);
  SET_SDL_KEY(DOWN, Down);
  SET_SDL_KEY(LEFT, Left);
  SET_SDL_KEY(RIGHT, Right);

  SET_SDL_KEY(A, A);
  SET_SDL_KEY(B, B);
  SET_SDL_KEY(C, C);
  SET_SDL_KEY(D, D);
  SET_SDL_KEY(E, E);
  SET_SDL_KEY(F, F);
  SET_SDL_KEY(G, G);
  SET_SDL_KEY(H, H);
  SET_SDL_KEY(I, I);
  SET_SDL_KEY(J, J);
  SET_SDL_KEY(K, K);
  SET_SDL_KEY(L, L);
  SET_SDL_KEY(M, M);
  SET_SDL_KEY(N, N);
  SET_SDL_KEY(O, O);
  SET_SDL_KEY(P, P);
  SET_SDL_KEY(Q, Q);
  SET_SDL_KEY(R, R);
  SET_SDL_KEY(S, S);
  SET_SDL_KEY(T, T);
  SET_SDL_KEY(U, U);
  SET_SDL_KEY(V, V);
  SET_SDL_KEY(W, W);
  SET_SDL_KEY(X, X);
  SET_SDL_KEY(Y, Y);
  SET_SDL_KEY(Z, Z);
  SET_SDL_KEY(0, 0);
  SET_SDL_KEY(1, 1);
  SET_SDL_KEY(2, 2);
  SET_SDL_KEY(3, 3);
  SET_SDL_KEY(4, 4);
  SET_SDL_KEY(5, 5);
  SET_SDL_KEY(6, 6);
  SET_SDL_KEY(7, 7);
  SET_SDL_KEY(8, 8);
  SET_SDL_KEY(9, 9);
  SET_SDL_KEY(PAGEUP, PageUp);
  SET_SDL_KEY(PAGEDOWN, PageDown);
  SET_SDL_KEY(HOME, Home);
  SET_SDL_KEY(END, End);
  SET_SDL_KEY(INSERT, Insert);
  SET_SDL_KEY(DELETE, Delete);
  SET_SDL_KEY(BACKSPACE, Backspace);
  SET_SDL_KEY(SPACE, Space);
  SET_SDL_KEY(RETURN, Enter);
  SET_SDL_KEY(ESCAPE, Escape);
  SET_SDL_KEY(TAB, Tab);

  auto mod_state = SDL_GetModState();
  input->keys_down[GET_KEY(Ctrl)] = mod_state & KMOD_CTRL;
  input->keys_down[GET_KEY(Alt)] = mod_state & KMOD_ALT;
  input->keys_down[GET_KEY(Shift)] = mod_state & KMOD_SHIFT;
  input->keys_down[GET_KEY(Super)] = mod_state & KMOD_GUI;

  // Set the control state
  // TODO(Cristian): This should not be here.
  if (input->keys_down[GET_KEY(Up)] || input->keys_down[GET_KEY(W)])
    input->up = true;
  if (input->keys_down[GET_KEY(Down)] || input->keys_down[GET_KEY(S)])
    input->down = true;
  if (input->keys_down[GET_KEY(Left)] || input->keys_down[GET_KEY(A)])
    input->left = true;
  if (input->keys_down[GET_KEY(Right)] || input->keys_down[GET_KEY(D)])
    input->right = true;
}

void HandleMouse(InputState* input) {
  auto mouse_state =
      SDL_GetMouseState(&input->mouse.pos.x, &input->mouse.pos.y);
  input->mouse.left = mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT);
  input->mouse.middle = mouse_state & SDL_BUTTON(SDL_BUTTON_MIDDLE);
  input->mouse.right = mouse_state & SDL_BUTTON(SDL_BUTTON_RIGHT);

  input->mouse_offset = input->mouse.pos - input->prev_mouse.pos;
}

void HandleMouseWheelEvent(const SDL_MouseWheelEvent* wheel_event,
                           InputState* input) {
  input->mouse.wheel.x = wheel_event->x;
  input->mouse.wheel.y = wheel_event->y;
}

void HandleWindowEvent(SDLVulkanWindow* sdl,
                       const SDL_WindowEvent* window_event) {
  // Fow now we're interested in window changed.
  if (window_event->event == SDL_WINDOWEVENT_SIZE_CHANGED) {
    sdl->width = window_event->data1;
    sdl->height = window_event->data2;
    sdl->events.push_back({WindowEvent::kWindowResize});
  }
}

// Vulkan API ------------------------------------------------------------------

std::vector<const char*> GetSDLVulkanInstanceExtensions(SDLVulkanWindow* sdl) {
  ASSERT(Valid(sdl));

  bool res;
  uint32_t count = 0;
  res = SDL_Vulkan_GetInstanceExtensions(sdl->window, &count, nullptr);
  if (!res) {
    LOG(ERROR) << "Could not get vulkan SDL extensions.";
    return {};
  }

  std::vector<const char*> extensions(count);
  res =
      SDL_Vulkan_GetInstanceExtensions(sdl->window, &count, extensions.data());
  if (!res) {
    LOG(ERROR) << "Could not get vulkan SDL extensions.";
    return {};
  }

  return extensions;
}

bool CreateSDLVulkanSurface(SDLVulkanWindow* sdl, VkInstance* instance,
                            VkSurfaceKHR* surface) {
  ASSERT(Valid(sdl));
  if (!SDL_Vulkan_CreateSurface(sdl->window, *instance, surface)) {
    LOG(ERROR) << "Could not create Vulkan SDL surface: " << SDL_GetError();
    return false;
  }

  return true;
}

}  // namespace

// Calling the implementations -------------------------------------------------

void SDLVulkanWindow::Init(Window* window) {
  InitSDLVulkan(this, window);
}

void SDLVulkanWindow::Shutdown() {
  ShutdownSDLVulkan(this);
}

std::pair<WindowEvent*, size_t> SDLVulkanWindow::NewFrame(Window* window,
                                                          InputState* input) {
  return NewFrameSDLVulkan(this, window, input);
}

std::vector<const char*> SDLVulkanWindow::GetVulkanInstanceExtensions() {
  return GetSDLVulkanInstanceExtensions(this);
}

bool SDLVulkanWindow::CreateVulkanSurface(void* vk_instance,
                                          void* surface_khr) {
  auto* instance = (VkInstance*)vk_instance;
  auto* surface = (VkSurfaceKHR*)surface_khr;
  return CreateSDLVulkanSurface(this, instance, surface);
}

}  // namespace sdl_vulkan
}  // namespace warhol
