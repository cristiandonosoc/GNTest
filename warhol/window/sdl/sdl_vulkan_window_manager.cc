// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/window/sdl/sdl_vulkan_window_manager.h"

#include <SDL2/SDL_vulkan.h>

#include "warhol/input/input.h"
#include "warhol/utils/assert.h"
#include "warhol/utils/log.h"
#include "warhol/window/window_manager.h"

// Sigh... Well done windows.
#ifdef max
#undef max
#endif

namespace warhol {

namespace {


bool Test(WindowManagerBackend* , uint64_t ) { return false; }


struct SetupInterface {
  SetupInterface() {
    WindowManagerBackend::Interface interface;

    /* interface.Init = sdl::InitSDLVulkan; */
    interface.Init = Test;
    interface.Shutdown = sdl::ShutdownSDL;

    interface.NewFrame = sdl::NewSDLFrame;
    interface.GetVulkanInstanceExtensions = sdl::GetVulkanInstanceExtensions;
    interface.CreateVulkanSurface = sdl::CreateVulkanSurface;

    SetWindowManagerBackendInterfaceTemplate(
        WindowManagerBackend::Type::kSDLVulkan, std::move(interface));
  }
};
SetupInterface setup_interface;

}  // namespace

namespace sdl {

namespace {

// Passes the info from the backend to the unified WindowManager interface.
void PassInfo(SDLVulkanWindowManager* from, WindowManager* to) {
  to->width = from->width;
  to->height = from->height;

  to->frame_delta = from->frame_delta;
  to->frame_delta_average = from->frame_delta_average;
  to->frame_rate = from->frame_rate;
  to->seconds = from->seconds;
}

}  // namespace

SDLVulkanWindowManager::SDLVulkanWindowManager() = default;
SDLVulkanWindowManager::~SDLVulkanWindowManager() = default;

// InitSDLVulkan ---------------------------------------------------------------

bool InitSDLVulkan(WindowManagerBackend* backend, uint64_t flags) {
  ASSERT(backend->window_manager);
  SDLVulkanWindowManager* sdl = new SDLVulkanWindowManager();
  backend->data = sdl;

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
    LOG(ERROR) << "Error loading SDL: " << SDL_GetError();
    return false;
  }

  sdl->window = SDL_CreateWindow("Warhol",
                                 SDL_WINDOWPOS_CENTERED,
                                 SDL_WINDOWPOS_CENTERED,
                                 1280,
                                 720,
                                 SDL_WINDOW_VULKAN | (uint32_t)flags);
  if (!sdl->window) {
    LOG(ERROR) << "Error creating window: " << SDL_GetError();
    return false;
  }
  SDL_GetWindowSize(sdl->window, &sdl->width, &sdl->height);
  PassInfo(sdl, backend->window_manager);

  return true;
}

// NewFrame --------------------------------------------------------------------

namespace {

void CalculateFramerate(SDLVulkanWindowManager* sdl);
void HandleKeyUpEvent(const SDL_KeyboardEvent*, InputState*);
void HandleKeysDown(InputState*);
void HandleMouse(InputState*);
void HandleMouseWheelEvent(const SDL_MouseWheelEvent*, InputState*);
void HandleWindowEvent(SDLVulkanWindowManager*, const SDL_WindowEvent*);

}  // namespace

std::pair<WindowEvent*, size_t>
NewSDLFrame(WindowManager* window, InputState* input) {
  WindowManagerBackend& backend = window->backend;
  ASSERT(backend.data);
  SDLVulkanWindowManager* sdl = (SDLVulkanWindowManager*)backend.data;

  sdl->events.clear();
  sdl->utf8_chars_inputted.clear();

  CalculateFramerate(sdl);
  InputState::InitFrame(input);   // We do the frame flip.

  // Handle events.
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT: sdl->events.push_back({WindowEvent::Type::kQuit});
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
  return {sdl->events.empty() ? nullptr : sdl->events.data(),
          sdl->events.size()};
}

namespace {

void CalculateFramerate(SDLVulkanWindowManager* sdl) {
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
      (sdl->frame_times_index + 1) % SDLVulkanWindowManager::kFrameTimesCounts;
  if (sdl->frame_delta_accum > 0.0) {
    sdl->frame_delta_average =
        sdl->frame_delta_accum / SDLVulkanWindowManager::kFrameTimesCounts;
  } else {
    sdl->frame_delta_average = std::numeric_limits<float>::max();
  }
  sdl->frame_rate = 1.0f / sdl->frame_delta_average;
}

void
HandleKeyUpEvent(const SDL_KeyboardEvent* key_event, InputState* input) {
  switch (key_event->keysym.scancode) {
    case SDL_SCANCODE_ESCAPE: input->keys_up[GET_KEY(Escape)] = true; break;
    default: break;
  }
}

#define SET_SDL_KEY(sdl_key, key)        \
  if (key_state[SDL_SCANCODE_##sdl_key]) \
    input->keys_down[GET_KEY(key)] = true;

void
HandleKeysDown(InputState* input) {
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
  auto mouse_state = SDL_GetMouseState(&input->mouse.pos.x,
                                       &input->mouse.pos.y);
  input->mouse.left = mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT);
  input->mouse.middle = mouse_state & SDL_BUTTON(SDL_BUTTON_MIDDLE);
  input->mouse.right = mouse_state & SDL_BUTTON(SDL_BUTTON_RIGHT);

  input->mouse_offset = input->mouse.pos - input->prev_mouse.pos;
}

void
HandleMouseWheelEvent(const SDL_MouseWheelEvent* wheel_event,
                      InputState* input) {
  input->mouse.wheel.x = wheel_event->x;
  input->mouse.wheel.y = wheel_event->y;
}

void HandleWindowEvent(SDLVulkanWindowManager* sdl,
                       const SDL_WindowEvent* window_event) {
  // Fow now we're interested in window changed.
  if (window_event->event == SDL_WINDOWEVENT_SIZE_CHANGED) {
    sdl->width = window_event->data1;
    sdl->height = window_event->data2;
    sdl->events.push_back({WindowEvent::Type::kWindowResize});
  }
}

}  // namespace

// Shutdown --------------------------------------------------------------------

void ShutdownSDL(WindowManagerBackend* backend) {
  if (!backend->valid())
    return;

  ASSERT(backend->data);
  SDLVulkanWindowManager* sdl = (SDLVulkanWindowManager*)backend->data;

  if (sdl->window)
    SDL_DestroyWindow(sdl->window);

  return;
}

// Vulkan API ------------------------------------------------------------------

std::vector<const char*> GetVulkanInstanceExtensions(WindowManager* window) {
  ASSERT(window->backend.valid());
  ASSERT(window->backend.type == WindowManagerBackend::Type::kSDLVulkan);
  auto* vulkan_sdl = (SDLVulkanWindowManager*)window->backend.data;

  bool res;
  uint32_t count = 0;
  res = SDL_Vulkan_GetInstanceExtensions(vulkan_sdl->window, &count, nullptr);
  if (!res)  {
    LOG(ERROR) << "Could not get vulkan SDL extensions.";
    return {};
  }

  std::vector<const char*> extensions(count);
  res = SDL_Vulkan_GetInstanceExtensions(vulkan_sdl->window, &count,
                                         extensions.data());
  if (!res)  {
    LOG(ERROR) << "Could not get vulkan SDL extensions.";
    return {};
  }

  return extensions;
}


bool CreateVulkanSurface(WindowManager* window, void* vk_instance,
                         void* surface_khr) {
  ASSERT(window->backend.valid());
  ASSERT(window->backend.type == WindowManagerBackend::Type::kSDLVulkan);
  auto* vulkan_sdl = (SDLVulkanWindowManager*)window->backend.data;

  VkInstance* instance = (VkInstance*)vk_instance;
  VkSurfaceKHR* surface = (VkSurfaceKHR*)surface_khr;
  if (!SDL_Vulkan_CreateSurface(vulkan_sdl->window, *instance, surface)) {
    LOG(ERROR) << "Could not create Vulkan SDL surface: " << SDL_GetError();
    return false;
  }

  return true;
}

}  // namespace sdl
}  // namespace warhol
