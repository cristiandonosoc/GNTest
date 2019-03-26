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
#include "warhol/utils/macros.h"

namespace warhol {
namespace opengl {

// Used to define a new variable name for each GL_CHECK call.
#define GL_VAR COMBINE(gl_err, __LINE__)

// Wraps an opengl call with an error checking query.
// Will assert on error.
#define GL_CHECK(opengl_call)                            \
  {                                                      \
    opengl_call;                                         \
    GLenum GL_VAR = glGetError();                        \
    if (GL_VAR != GL_NO_ERROR) {                         \
      auto location = FROM_HERE;                         \
      fprintf(stderr,                                    \
              "[ERROR][%s:%d] When calling %s: %s\n",    \
              location.file,                             \
              location.line,                             \
              #opengl_call,                              \
              ::warhol::opengl::GLEnumToString(GL_VAR)); \
      NOT_REACHED("Invalid OpenGL call. See logs.");     \
    }                                                    \
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

  bool Init(Renderer*, Window*) override;
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
