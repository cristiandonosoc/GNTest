// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <vector>

#include "warhol/window/sdl/def.h"
#include "warhol/utils/macros.h"
#include "warhol/window/common/window_manager_backend.h"

namespace warhol {

struct InputState;
struct WindowEvent;
struct WindowManager;

namespace sdl {


struct SDLVulkanWindowManager : public WindowManagerBackend {
  // Amount of frames to keep track of in order to get an average frame time.
  static constexpr int kFrameTimesCounts = 128;

  SDLVulkanWindowManager();
  ~SDLVulkanWindowManager();
  DELETE_COPY_AND_ASSIGN(SDLVulkanWindowManager);
  DELETE_MOVE_AND_ASSIGN(SDLVulkanWindowManager);

  SDL_Window* window = nullptr;

  int width = 0;
  int height = 0;

  // Total time since the start of the game.
  uint64_t total_time = 0;
  float seconds = 0;
  float frame_delta = 0;

  float frame_delta_accum = 0;  // The accumulated average.
  float frame_delta_average = 0;
  float frame_rate = 0;

  // TODO(Cristian): When we're interested, start tracking these times.
  float frame_times[kFrameTimesCounts];
  int frame_times_index = 0;

  std::vector<WindowEvent> events;
  std::vector<char> utf8_chars_inputted;

  // Interface -----------------------------------------------------------------

  void Init(WindowManager*, uint64_t flags) override;
  void Shutdown() override;
  std::pair<WindowEvent*, size_t> NewFrame(InputState*) override;
  std::vector<const char*> GetVulkanInstanceExtensions() override;
  bool CreateVulkanSurface(void* vk_instance, void* surface_khr) override;
};

}  // namespace sdl
}  // namespace warhol
