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

void HandleWindowEvents(ApplicationContext* app_context, WindowEvent* events,
                        size_t event_count) {
  for (size_t i = 0; i < event_count; i++) {
    WindowEvent& event = events[i];
    if (event == WindowEvent::kQuit) {
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
    InitWindowManager(&window, WindowBackendType::kSDLVulkan);

    float timing = timer.End();
    LOG(INFO) << "Created SDL context: " << timing << " ms.";
  }

  {
    Timer timer = Timer::ManualTimer();

    renderer.window = &window;
    InitRenderer(&renderer, RendererType::kVulkan, &window);

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
    if (auto [events, event_count] = UpdateWindowManager(&window, &input);
        events != nullptr) {
      HandleWindowEvents(&app_context, events, event_count);
    }

    if (input.keys_up[GET_KEY(Escape)])
      break;

    DrawFrame(&renderer, &camera);
    SDL_Delay(10);
  }
}
