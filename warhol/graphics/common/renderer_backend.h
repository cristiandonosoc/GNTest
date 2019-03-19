// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdlib.h>

#include <memory>

#include "warhol/utils/macros.h"

namespace warhol {

struct Camera;
struct Mesh;
struct ShaderManager;
struct RenderCommand;
struct Renderer;

struct RendererBackend {
  // DO NOT add numbers to this enum.
  enum class Type : uint32_t {
    kVulkan,
    kLast,  // DO NOT specialize with numbers.
  };
  static const char* TypeToString(Type);

  RendererBackend();
  RendererBackend(Type);
  virtual ~RendererBackend();

  bool valid() const { return renderer != nullptr; }

  Type type = Type::kLast;
  Renderer* renderer;  // Not-owning.

  // Interface -----------------------------------------------------------------

  virtual void Init(Renderer*) = 0;
  virtual void Shutdown() = 0;
  virtual void ExecuteCommands(RenderCommand*, size_t command_count) = 0;
  virtual void DrawFrame(Camera*) = 0;

  // Loads the mesh into the GPU.
  virtual void LoadMesh(Mesh*) = 0;
  virtual void UnloadMesh(Mesh*) = 0;

  virtual ShaderManager* GetShaderManager() = 0;
};

// Backend Suscription ---------------------------------------------------------

// Each backend, upon application startup, must suscribe a function that will
// be called to create a that particular RendererBackend.
using RendererBackendFactory = std::unique_ptr<RendererBackend> (*)();
void SuscribeRendererBackendFactory(RendererBackend::Type,
                                         RendererBackendFactory);

std::unique_ptr<RendererBackend>
CreateRendererBackend(RendererBackend::Type);

}  // namespace warhol
