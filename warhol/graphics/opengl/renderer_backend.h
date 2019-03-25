// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "warhol/graphics/common/renderer_backend.h"

#include <stdio.h>

#include <map>

#include <GL/gl3w.h>

#include "warhol/graphics/opengl/utils.h"
#include "warhol/utils/assert.h"
#include "warhol/utils/location.h"

namespace warhol {
namespace opengl {

#define GL_CALL(func, ...) \
  ::warhol::opengl::InternalOpenGLCall(func, #func, FROM_HERE, __VA_ARGS__)

template <typename FunctionType, typename... Args>
static inline void InternalOpenGLCall(FunctionType func,
                                  const char* func_str,
                                  const Location& location,
                                  Args&&... args) {
  func(std::forward<Args>(args)...);
  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    fprintf(stderr,
            "[ERROR][%s:%d] When calling %s: %s\n",
            location.file,
            location.line,
            func_str,
            GLEnumToString(error));
    ASSERT(false);
  }
}

struct MeshHandles {
  uint32_t vbo = 0;
  uint32_t ebo = 0;
  uint32_t vao = 0;
};


struct OpenGLRendererBackend : public RendererBackend {
  OpenGLRendererBackend() = default;
  ~OpenGLRendererBackend();   // RAII "semantics".
  DELETE_COPY_AND_ASSIGN(OpenGLRendererBackend);
  DEFAULT_MOVE_AND_ASSIGN(OpenGLRendererBackend);

  bool loaded = false;

  // Maps from external resource UUID to internal objects.
  std::map<uint64_t, uint32_t> loaded_shaders;
  std::map<uint64_t, MeshHandles> loaded_meshes;
  std::map<uint64_t, uint32_t> loaded_textures;

  // Buffer that holds the camera matrices.
  uint32_t camera_ubo = 0;
  // If the last camera was already set, we don't need to re-send the uniforms.
  Camera* last_set_camera = nullptr;

  // Virtual interface ---------------------------------------------------------

  bool Init(Renderer*) override;
  void Shutdown() override;
  void ExecuteCommands(RenderCommand*, size_t command_count) override;
  bool StageShader(Shader*) override;
  void UnstageShader(Shader*) override;
  bool StageMesh(Mesh*) override;
  void UnstageMesh(Mesh*) override;
  bool StageTexture(Texture*) override;
  void UnstageTexture(Texture*) override;
  void StartFrame(Renderer*) override;
  void ExecuteCommands(Renderer*, LinkedList<RenderCommand>*) override;
  void EndFrame(Renderer*) override;
};

inline bool Valid(OpenGLRendererBackend* opengl) { return opengl->loaded; }

}  // namespace opengl
}  // namespace warhol
