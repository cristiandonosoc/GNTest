// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <iostream>
#include <optional>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include <warhol/debug/timer.h>
#include <warhol/graphics/renderer.h>
#include <warhol/input/input.h>
#include <warhol/scene/camera.h>
#include <warhol/window/window_manager.h>
#include <warhol/utils/glm_impl.h>
#include <warhol/utils/log.h>

using namespace warhol;

namespace {

struct ApplicationContext {
  bool running = false;
  bool window_size_changed = false;
};

/* bool Update(const SDLContext& context, UBO* ubo) { */
/*   ubo->model = glm::rotate( */
/*       glm::mat4{1.0f}, time * glm::radians(90.0f), glm::vec3{0.0f, 0.0f, 1.0f}); */
/*   return true; */
/* } */

void HandleWindowEvents(ApplicationContext* app_context, WindowEvent* events,
                        size_t event_count) {
  for (size_t i = 0; i < event_count; i++) {
    WindowEvent& event = events[i];
    if (event.type == WindowEvent::Type::kQuit) {
      app_context->running = false;
      break;
    }
  }
}

}  // namespace

int main() {
  ApplicationContext app_context = {};
  WindowManager window = {};
  Renderer renderer = {};
  Camera camera = {};

  {
    Timer timer = Timer::ManualTimer();

    if (!InitWindowManager(&window, WindowManagerBackend::Type::kSDLVulkan,
                           SDL_WINDOW_RESIZABLE))
      return 1;

    float timing = timer.End();
    LOG(INFO) << "Created SDL context: " << timing << " ms.";
  }

  {
    Timer timer = Timer::ManualTimer();

    renderer.window = &window;
    renderer.backend_type = Renderer::BackendType::kVulkan;
    if (!InitRenderer(&renderer)) {
      LOG(ERROR) << "Could not setup vulkan. Exiting.";
      return 1;
    }

    float timing = timer.End();
    LOG(INFO) << "Initialized vulkan: " << timing << " ms.";
  }

  LOG(INFO) << "Window size. WIDTH: " << window.width
            << ", HEIGHT: " << window.height;

  camera.view = glm::lookAt(glm::vec3{2.0f, 2.0f, 2.0f}, {}, {0, 0, 0.1f});
  camera.projection =
      glm::perspective(glm::radians(45.0f),
                       (float)window.width / (float)window.height,
                       0.1f, 100.f);

  InputState input = InputState::Create();
  app_context.running = true;
  while (app_context.running) {
    if (auto [events, event_count] = NewFrame(&window, &input);
        events != nullptr) {
      HandleWindowEvents(&app_context, events, event_count);
    }

    if (input.keys_up[GET_KEY(Escape)])
      break;

    /* if (!Update(sdl_context, &ubo)) */
    /*   break; */

    if (!DrawFrame(&renderer, &camera)) {
      LOG(ERROR) << "Error drawing with vulkan. Exiting.";
      break;
    }

    SDL_Delay(10);
  }

  if (!ShutdownRenderer(&renderer)) {
    LOG(ERROR) << "Could not shutdown renderer.";
    exit(1);
  }
}
