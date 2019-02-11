// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "warhol/utils/macros.h"

namespace warhol {

struct Camera;
struct Renderer;

struct RendererBackend {
  bool valid() const { return renderer != nullptr && data != nullptr; }

  RendererBackend();
  ~RendererBackend();
  DELETE_COPY_AND_ASSIGN(RendererBackend);
  DECLARE_MOVE_AND_ASSIGN(RendererBackend);

  Renderer* renderer;   // Not-owning.

  // IMPORTANT: If you add more functions, remember to update the move ctor!
  bool (*InitFunction)(RendererBackend*) = nullptr;
  bool (*ExecuteCommands)(RendererBackend*) = nullptr;
  bool (*ShutdownFunction)(RendererBackend*) = nullptr;
  bool (*DrawFrameFunction)(RendererBackend*, Camera*) = nullptr;

  // |data| lifecycle is managed by Init and Shutdown. Shutdown is called on the
  // destructor.
  void* data = nullptr;
};

void Clear(RendererBackend*);

}  // namespace warhol
