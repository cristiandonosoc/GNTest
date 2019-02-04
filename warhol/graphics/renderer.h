// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdlib.h>

#include <memory>

#include "warhol/utils/macros.h"

namespace warhol {

class SDLContext;

struct Camera;
struct Renderer;  // Defined later in file.

struct BackendInterface {
  bool valid() const { return renderer != nullptr && data != nullptr; }

  BackendInterface();
  ~BackendInterface();
  DELETE_COPY_AND_ASSIGN(BackendInterface);
  DECLARE_MOVE_AND_ASSIGN(BackendInterface);

  Renderer* renderer;   // Not-owning.

  // IMPORTANT: If you add more functions, remember to update the move ctor!
  bool (*InitFunction)(BackendInterface*) = nullptr;
  bool (*ShutdownFunction)(BackendInterface*) = nullptr;
  bool (*DrawFrameFunction)(BackendInterface*, Camera*) = nullptr;

  void* data = nullptr;
};

void Clear(BackendInterface*);


struct Renderer {
  enum class BackendType : uint32_t {
    kVulkan,
    kLast,
  };
  const char* BackendTypeToString(BackendType);

  enum class WindowManager : uint32_t {
    kSDL,
    kLast,
  };
  const char* WindowManagerToString(WindowManager);

  Renderer();
  ~Renderer();
  DELETE_COPY_AND_ASSIGN(Renderer);
  DELETE_MOVE_AND_ASSIGN(Renderer);

  BackendType backend_type = BackendType::kLast;
  WindowManager window_manager = WindowManager::kLast;

  SDLContext* sdl_context = nullptr;
  BackendInterface backend_interface = {};
};

bool InitRenderer(Renderer*);
bool ShutdownRenderer(Renderer*);

void WindowSizeChanged(Renderer*, uint32_t width, uint32_t height);
bool DrawFrame(Renderer*, Camera*);

}  // namespace
