// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <memory>

#include "warhol/utils/macros.h"

namespace warhol {

class SDLContext;
struct BackendInterface;
struct Camera;
struct Renderer;

namespace vulkan {

struct Context;

struct RendererBackend {
  RendererBackend();
  ~RendererBackend();
  DELETE_COPY_AND_ASSIGN(RendererBackend);
  DELETE_MOVE_AND_ASSIGN(RendererBackend);

  std::unique_ptr<vulkan::Context> context;
};

bool InitRendererBackend(BackendInterface*);
bool ExecuteCommands(BackendInterface*);
bool ShutdownRendererBackend(BackendInterface*);
bool DrawFrame(BackendInterface*, Camera*);

}  // namespace vulkan
}  // namespace warhol
