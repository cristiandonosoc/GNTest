// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdlib.h>

#include "warhol/graphics/common/render_command.h"

namespace warhol {

struct BasePaths;
struct Mesh;
struct Renderer;
struct Shader;
struct StageTextureConfig;
struct Texture;
struct Window;

// Abstract interface all graphics backends must implement.

struct RendererBackend {
  virtual ~RendererBackend() = default;

  // Virtual interface.

  virtual bool Init(Renderer*, Window*) = 0;
  virtual void Shutdown() = 0;

  virtual bool StageMesh(Mesh*) = 0;
  virtual void UnstageMesh(Mesh*) = 0;
  virtual bool IsMeshStaged(Mesh*) = 0;

  virtual bool UploadMeshRange(Mesh*,
                               IndexRange vertex_range = {},
                               IndexRange index_range = {}) = 0;

  // Shader --------------------------------------------------------------------

  virtual bool ParseShader(Renderer*, BasePaths*,
                           const std::string& vert_name,
                           const std::string& frag_name,
                           Shader* out) = 0;
  virtual bool StageShader(Shader*) = 0;
  virtual void UnstageShader(Shader*) = 0;
  virtual bool IsShaderStaged(Shader*) = 0;

  virtual bool StageTexture(Texture*, StageTextureConfig*) = 0;
  virtual void UnstageTexture(Texture*) = 0;
  virtual bool IsTextureStaged(Texture*) = 0;

  virtual void StartFrame(Renderer*) = 0;
  virtual void ExecuteCommands(Renderer*, List<RenderCommand>*) = 0;
  virtual void EndFrame(Renderer*) = 0;
};

}  // namespace warhol
