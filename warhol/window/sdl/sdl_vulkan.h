// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <vector>

#include "warhol/window/sdl/def.h"
#include "warhol/utils/macros.h"
#include "warhol/window/common/window_backend.h"

namespace warhol {
namespace sdl_vulkan {

struct SDLVulkanWindow : public WindowBackend {
  // Amount of frames to keep track of in order to get an average frame time.
  static constexpr int kFrameTimesCounts = 128;

  SDLVulkanWindow() = default;
  ~SDLVulkanWindow();
  DELETE_COPY_AND_ASSIGN(SDLVulkanWindow);
  DELETE_MOVE_AND_ASSIGN(SDLVulkanWindow);

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

  bool Init(Window*) override;
  void Shutdown() override;
  std::pair<WindowEvent*, size_t> NewFrame(Window*, InputState*) override;
  std::vector<const char*> GetVulkanInstanceExtensions() override;
  bool CreateVulkanSurface(void* vk_instance, void* surface_khr) override;
};

inline bool Valid(SDLVulkanWindow* w) { return w->window != nullptr; }

}  // namespace sdl_vulkan
}  // namespace warhol
