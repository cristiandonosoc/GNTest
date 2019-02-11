// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdlib.h>

#include <utility>
#include <vector>

#include "warhol/utils/macros.h"

namespace warhol {

struct InputState;
struct WindowManager;   // Defined later in file.

struct WindowEvent {
  enum class Type {
    kQuit,
    kWindowResize,
    kLast,
  };
  static const char* TypeToString(Type);

  Type type = Type::kLast;
};


struct WindowManagerBackend {
  bool valid() const { return window_manager != nullptr && data != nullptr; }

  WindowManagerBackend();
  ~WindowManagerBackend();
  DELETE_COPY_AND_ASSIGN(WindowManagerBackend);
  DECLARE_MOVE_AND_ASSIGN(WindowManagerBackend);

  std::pair<WindowEvent*, size_t>
  (*NewFrameFunction)(WindowManagerBackend*, InputState*) = nullptr;
  void (*ShutdownFunction)(WindowManagerBackend*) = nullptr;

  WindowManager* window_manager = nullptr;
  void* data = nullptr;  // Underlying memory of backend.
};

struct WindowManager {
  static constexpr size_t kRollingAverageFrames = 60;

  enum class Type {
    kSDLVulkan,
    kLast,
  };
  static const char* TypeToString(Type);

  Type type = Type::kLast;
  WindowManagerBackend backend;

  size_t width = 0;
  size_t height = 0;

  float frame_delta = 0;          // Delta within the last frame in seconds.
  float frame_delta_average = 0;  // Rolling average over kRollingAverageFrames.
  float frame_rate = 0;           // 1 / frame_delta_average.
  float seconds = 0;              // Seconds since Init was called.
};

// WindowManager must be already set with |type|.
bool InitWindowManager(WindowManager*, uint64_t flags);
std::pair<WindowEvent*, size_t> NewFrame(WindowManager*, InputState*);

// *** VULKAN ONLY ***

std::vector<const char*> GetVulkanInstanceExtensions(WindowManager*);

// Will be casted to the right type in the .cc
// This is so that we don't need to typedef the values and we don't create
// unnecessary dependencies on the graphics libraries.
bool CreateVulkanSurface(WindowManager*, void* vk_instance, void* surface_khr);

}  // namespace warhol
