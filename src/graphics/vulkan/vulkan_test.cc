// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <stdio.h>

#include <memory>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "src/graphics/vulkan/vulkan_context.h"

#include "utils/file.h"
#include "utils/log.h"

void FreeSDL(SDL_Window* window) {
  LOG(DEBUG) << "Freeing SDL";
  if (window) {
    SDL_DestroyWindow(window);
  }
  // TODO: Use out of scope runner for this.
  SDL_Quit();
}

void MainLoop(SDL_Window*, warhol::VulkanContext*);
void DrawFrame(warhol::VulkanContext*);

int main() {
  // Setup SDL2.
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
    LOG(ERROR) << "Loading SDL: " << SDL_GetError();
    return 1;
  }

  // Data about displays.
  LOG(INFO) << "Information from SDL:";
  LOG(INFO) << "- Amount of displays: " << SDL_GetNumVideoDisplays();

  // Create SDL window.
  SDL_Window* window_ptr = SDL_CreateWindow("Example", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, 640, 480,
                       SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);
  std::unique_ptr<SDL_Window, void(*)(SDL_Window*)> window(window_ptr, FreeSDL);
  if (!window) {
    LOG(ERROR) << "Creating SDL2 window: " << SDL_GetError();
    return 1;
  }

  warhol::VulkanContext vulkan_context = {};
  warhol::Status res = vulkan_context.Init(window.get());
  if (!res.ok()) {
    LOG (ERROR) << "Initializing Vulkan: " << res.err_msg();
    return 1;
  }

  MainLoop(window.get(), &vulkan_context);

  LOG(INFO) << "Correctly initialized vulkan";
  return 0;
}


void MainLoop(SDL_Window*, warhol::VulkanContext*) {
  bool running = true;
  while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
      }
    }
    break;

    SDL_Delay(10);  // Pause for 10 ms.
  }

}
