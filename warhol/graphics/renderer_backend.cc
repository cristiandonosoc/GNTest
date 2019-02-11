// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/renderer_backend.h"

#include "warhol/utils/assert.h"

namespace warhol {

// RendererBackend -------------------------------------------------------------

RendererBackend::RendererBackend() = default;
RendererBackend::~RendererBackend() {
  if (valid()) {
    ASSERT(data);
    ShutdownFunction(this);   // Frees data.
  }
  Clear(this);
}

RendererBackend::RendererBackend(RendererBackend&& other)
    : renderer(other.renderer),
      InitFunction(other.InitFunction),
      ExecuteCommands(other.ExecuteCommands),
      ShutdownFunction(other.ShutdownFunction),
      DrawFrameFunction(other.DrawFrameFunction),
      data(other.data) {
  Clear(&other);
}

RendererBackend& RendererBackend::operator=(RendererBackend&& other) {
  renderer = other.renderer;
  InitFunction = other.InitFunction;
  ExecuteCommands = other.ExecuteCommands;
  ShutdownFunction = other.ShutdownFunction;
  DrawFrameFunction = other.DrawFrameFunction;
  data = other.data;
  Clear(&other);
  return *this;
}

void Clear(RendererBackend* bi) {
  bi->renderer = nullptr;
  bi->InitFunction = nullptr;
  bi->ExecuteCommands = nullptr;
  bi->ShutdownFunction = nullptr;
  bi->DrawFrameFunction = nullptr;
  bi->data = nullptr;
}

}  // namespace warhol
