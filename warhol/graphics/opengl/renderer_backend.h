// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "warhol/graphics/common/renderer_backend.h"

#include <stdio.h>

#include <map>

#include <GL/gl3w.h>

#include "warhol/graphics/opengl/shader.h"
#include "warhol/graphics/opengl/utils.h"
#include "warhol/utils/clear_on_move.h"
#include "warhol/utils/location.h"
#include "warhol/utils/macros.h"

namespace warhol {
namespace opengl {

struct MeshHandles {
  uint32_t vbo = 0;
  uint32_t ebo = 0;
  uint32_t vao = 0;
};

struct TextureHandles {
  uint32_t tex_handle = 0;
};

struct OpenGLRendererBackend : public RendererBackend {
  OpenGLRendererBackend() = default;
  ~OpenGLRendererBackend();   // RAII "semantics".
  DELETE_COPY_AND_ASSIGN(OpenGLRendererBackend);
  DEFAULT_MOVE_AND_ASSIGN(OpenGLRendererBackend);

  // Maps from external resource UUID to internal objects.
  std::map<uint64_t, ShaderHandles> loaded_shaders;
  std::map<uint64_t, MeshHandles> loaded_meshes;
  std::map<uint64_t, TextureHandles> loaded_textures;

  // Buffer that holds the camera matrices.
  ClearOnMove<uint32_t> camera_ubo = 0;

  // If the last camera was already set, we don't need to re-send the uniforms.
  Camera* last_set_camera = nullptr;

  bool loaded = false;

  // Virtual interface ---------------------------------------------------------

  bool Init(Renderer*, Window*) override;
  void Shutdown() override;

  bool StageMesh(Mesh*) override;
  void UnstageMesh(Mesh*) override;
  bool IsMeshStaged(Mesh*) override;

  bool UploadMeshRange(Mesh*,
                       IndexRange vertex_range = {},
                       IndexRange index_range = {}) override;

  bool StageShader(Shader*) override;
  void UnstageShader(Shader*) override;
  bool IsShaderStaged(Shader*) override;

  bool StageTexture(Texture*, StageTextureConfig*) override;
  void UnstageTexture(Texture*) override;
  bool IsTextureStaged(Texture*) override;

  void StartFrame(Renderer*) override;
  void ExecuteCommands(Renderer*, List<RenderCommand>*) override;
  void EndFrame(Renderer*) override;
};

inline bool Valid(OpenGLRendererBackend* opengl) { return opengl->loaded; }

}  // namespace opengl
}  // namespace warhol
