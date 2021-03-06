// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/opengl/renderer_backend.h"

#include "warhol/graphics/common/mesh.h"
#include "warhol/graphics/common/renderer.h"
#include "warhol/graphics/common/shader.h"
#include "warhol/graphics/common/texture.h"
#include "warhol/graphics/opengl/mesh.h"
#include "warhol/graphics/opengl/shader.h"
#include "warhol/graphics/opengl/texture.h"
#include "warhol/scene/camera.h"
#include "warhol/utils/debug.h"
#include "warhol/utils/log.h"
#include "warhol/utils/macros.h"

namespace warhol {
namespace opengl {

// OpenGL Debug Callback -------------------------------------------------------

#ifdef DEBUG_MODE

namespace {

void APIENTRY OpenGLDebugCallback(GLenum source, GLenum type, GLuint id,
                                  GLenum severity, GLsizei length,
                                  const GLchar* message,
                                  const void* user_param) {
  (void)user_param;
  (void)length;
  if (type == GL_DEBUG_TYPE_OTHER)
    return;

  std::stringstream ss;

  ss << std::endl;
  ss << "---------------------opengl-callback-start------------" << std::endl;
  ss << "Source: " << GLEnumToString(source) << std::endl;
  ss << "message: " << message << std::endl;
  ss << "type: ";
  switch (type) {
    case GL_DEBUG_TYPE_ERROR: ss << "ERROR"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: ss << "DEPRECATED_BEHAVIOR"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: ss << "UNDEFINED_BEHAVIOR"; break;
    case GL_DEBUG_TYPE_PORTABILITY: ss << "PORTABILITY"; break;
    case GL_DEBUG_TYPE_PERFORMANCE: ss << "PERFORMANCE"; break;
    case GL_DEBUG_TYPE_OTHER: ss << "OTHER"; break;
  }
  ss << std::endl;

  ss << "id: " << id << std::endl;
  ss << "severity: ";
  switch (severity) {
    case GL_DEBUG_SEVERITY_LOW: ss << "LOW"; break;
    case GL_DEBUG_SEVERITY_MEDIUM: ss << "MEDIUM"; break;
    case GL_DEBUG_SEVERITY_HIGH: ss << "HIGH"; break;
    default: ss << "<unknown>"; break;
  }
  ss << std::endl;
  ss << "---------------------opengl-callback-end--------------";

  LOG(ERROR) << ss.str();
}

}  // namespace

#endif

// Backend Suscription ---------------------------------------------------------

namespace {

std::unique_ptr<RendererBackend> CreateOpenGLRenderer() {
  return std::make_unique<OpenGLRendererBackend>();
}

struct BackendSuscriptor {
  BackendSuscriptor() {
    SuscribeRendererBackendFactory(RendererType::kOpenGL, CreateOpenGLRenderer);
  }
};

// Trigger the suscription.
BackendSuscriptor backend_suscriptor;

} // namespace

// Init ------------------------------------------------------------------------

namespace {

const char*
Gl3wInitResultToString(int res) {
  switch (res) {
    case GL3W_OK: return "GL3W_OK";
    case GL3W_ERROR_INIT: return "GL3W_ERROR_INIT";
    case GL3W_ERROR_LIBRARY_OPEN: return "GL3W_ERROR_LIBRARY_OPEN";
    case GL3W_ERROR_OPENGL_VERSION: return "GL3W_ERROR_OPENGL_VERSION";
    default:
      break;
  }
  LOG(WARNING) << "Got unknown GL3W init result: " << res;
  return "";
}

bool OpenGLInit(OpenGLRendererBackend* opengl) {
  int res = gl3wInit();
  if (res != GL3W_OK) {
    LOG(WARNING) << "Got non-OK GL3W result: " << Gl3wInitResultToString(res);
    return false;
  }

  uint32_t camera_ubo;
  glGenBuffers(1, &camera_ubo);
  opengl->camera_ubo = camera_ubo;

  glBindBuffer(GL_UNIFORM_BUFFER, opengl->camera_ubo.value);
  glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, NULL);

  glBindBufferBase(GL_UNIFORM_BUFFER, 0, opengl->camera_ubo.value);

#if DEBUG_MODE
  if (glDebugMessageCallback) {
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(OpenGLDebugCallback, nullptr);

    GLuint unused_ids = 0;
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0,
                          &unused_ids, true);
  }
#endif

  opengl->loaded = true;
  return true;
}

}  // namespace

bool OpenGLRendererBackend::Init(Renderer*, Window*) {
  return OpenGLInit(this);
}

// Shutdown --------------------------------------------------------------------

namespace {

void OpenGLShutdown(OpenGLRendererBackend* opengl) {
  ASSERT(Valid(opengl));

  for (auto& [shader_uuid, handles] : opengl->loaded_shaders) {
    DeleteShaderHandles(&handles);
  }
  opengl->loaded_shaders.clear();

  for (auto& [mesh_uuid, handles] : opengl->loaded_meshes) {
    DeleteMeshHandles(&handles);
  }
  opengl->loaded_meshes.clear();

  if (opengl->camera_ubo.has_value()) {
    glDeleteBuffers(1, &opengl->camera_ubo.value);
    opengl->camera_ubo = 0;
  }
  opengl->camera_ubo.clear();

  opengl->loaded = false;
}

}  // namespace

void OpenGLRendererBackend::Shutdown() {
  return OpenGLShutdown(this);
}

// Mesh Handling ---------------------------------------------------------------

bool OpenGLRendererBackend::StageMesh(Mesh* mesh) {
  return OpenGLStageMesh(this, mesh);
}

inline bool OpenGLRendererBackend::IsMeshStaged(Mesh* mesh) {
  ASSERT(Valid(this));
  return this->loaded_meshes.count(mesh->uuid.value) > 0;
}

bool OpenGLRendererBackend::UploadMeshRange(Mesh* mesh,
                                            IndexRange vert_range,
                                            IndexRange index_range) {
  return OpenGLRendererUploadMeshRange(this, mesh, vert_range, index_range);
}

void OpenGLRendererBackend::UnstageMesh(Mesh* mesh) {
  OpenGLUnstageMesh(this, mesh);
}

// Shader Handling -------------------------------------------------------------

bool OpenGLRendererBackend::ParseShader(Renderer*,
                                        BasePaths* paths,
                                        const std::string& vert_name,
                                        const std::string& frag_name,
                                        Shader* out) {
  return OpenGLParseShader(paths, vert_name, frag_name, out);
}

bool OpenGLRendererBackend::StageShader(Shader* shader) {
  return OpenGLStageShader(this, shader);
}

bool OpenGLRendererBackend::IsShaderStaged(Shader* shader) {
  ASSERT(Valid(this));
  return this->loaded_shaders.count(shader->uuid.value) > 0;
}

void OpenGLRendererBackend::UnstageShader(Shader* shader) {
  OpenGLUnstageShader(this, shader);
}

// Texture Handling ------------------------------------------------------------

bool OpenGLRendererBackend::StageTexture(Texture* texture,
                                         StageTextureConfig* config) {
  return OpenGLStageTexture(this, texture, config);
}

bool OpenGLRendererBackend::IsTextureStaged(Texture* texture) {
  ASSERT(Valid(this));
  return this->loaded_textures.count(texture->uuid.value) > 0;
}

void OpenGLRendererBackend::UnstageTexture(Texture* texture) {
  OpenGLUnstageTexture(this, texture);
}

// Start Frame -----------------------------------------------------------------

namespace {

void OpenGLStartFrame(Renderer* renderer) {
  glClearColor(renderer->clear_color.x,
               renderer->clear_color.y,
               renderer->clear_color.z,
               1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

}  // namespace

void OpenGLRendererBackend::StartFrame(Renderer* renderer) {
  OpenGLStartFrame(renderer);
}

// Execute Commands ------------------------------------------------------------

namespace {

void SetCameraMatrices(OpenGLRendererBackend* opengl, Camera* camera) {
  if (opengl->last_set_camera == camera)
    return;

  void* data = &camera->projection;
  glBindBuffer(GL_UNIFORM_BUFFER, opengl->camera_ubo.value);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, 2 * sizeof(glm::mat4), data);
  glBindBuffer(GL_UNIFORM_BUFFER, NULL);

  opengl->last_set_camera = camera;
}

void SetUniforms(Shader* shader, ShaderHandles* handles,
                 MeshRenderAction* action) {
  if (handles->vert_ubo_binding > -1) {
    ASSERT(handles->vert_ubo_handle > 0);
    ASSERT(action->vert_uniforms);

    glBindBuffer(GL_UNIFORM_BUFFER, handles->vert_ubo_handle);
    glBufferData(GL_UNIFORM_BUFFER,
                 shader->vert_ubo_size,
                 action->vert_uniforms,
                 GL_STREAM_DRAW);
  }

  if (handles->frag_ubo_binding > -1) {
    ASSERT(handles->frag_ubo_handle > 0);
    ASSERT(action->frag_uniforms);

    glBindBuffer(GL_UNIFORM_BUFFER, handles->frag_ubo_handle);
    glBufferData(GL_UNIFORM_BUFFER,
                 shader->frag_ubo_size,
                 action->frag_uniforms,
                 GL_STREAM_DRAW);
  }

  glBindBuffer(GL_UNIFORM_BUFFER, NULL);
}

#define SET_GL_CONFIG(flag, gl_name) \
  if (flag) {                           \
    glEnable(gl_name);                  \
  } else {                              \
    glDisable(gl_name);                 \
  }

void SetConfigs(RenderCommandConfig* config) {
  if (config->blend_enabled) {
    glEnable(GL_BLEND);

    // TODO(Cristian): Have a way of setting the blend function!!!!!
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  } else {
    glDisable(GL_BLEND);
  }

  SET_GL_CONFIG(config->cull_faces, GL_CULL_FACE);
  SET_GL_CONFIG(config->depth_test, GL_DEPTH_TEST);
  SET_GL_CONFIG(config->scissor_test, GL_SCISSOR_TEST);
}

void ExecuteMeshActions(OpenGLRendererBackend* opengl,
                        RenderCommand* command,
                        ShaderHandles* shader_handles) {
  for (MeshRenderAction& action : command->mesh_actions) {
    size_t size = GetSize(action.index_range);
    size_t offset = GetOffset(action.index_range);
    /* LOG(DEBUG) << ToString(action.index_range); */
    if (size == 0)
      continue;

    auto mesh_it = opengl->loaded_meshes.find(action.mesh->uuid.value);
    ASSERT(mesh_it != opengl->loaded_meshes.end());

    SetUniforms(command->shader, shader_handles, &action);

    MeshHandles& handles = mesh_it->second;
    glBindVertexArray(handles.vao);

    for (int i = 0; i < command->shader->texture_count; i++) {
      Texture* texture = action.textures + i;
      auto tex_it = opengl->loaded_textures.find(texture->uuid.value);
      ASSERT(tex_it != opengl->loaded_textures.end());

      uint32_t tex_handle = tex_it->second.tex_handle;
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, tex_handle);
    }

    // Set scissoring. We check if the rectangle is non zero.
    if (command->config.scissor_test && Sum(action.scissor) > 0) {
      glScissor((int)action.scissor.x, (int)action.scissor.y,
                (int)action.scissor.z, (int)action.scissor.w);
    }

    glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, (void*)offset);
  }
  glBindVertexArray(NULL);
}

void ValidateRenderCommands(List<RenderCommand>* commands) {
  for (auto& command : *commands) {
    ASSERT(command.camera);
    ASSERT(command.shader);
    ASSERT(command.type == RenderCommandType::kMesh);
    for (auto& action : command.mesh_actions) {
      ASSERT(action.mesh);
    }
  }
}

void OpenGLExecuteCommands(OpenGLRendererBackend* opengl,
                           List<RenderCommand>* commands) {
#if DEBUG_MODE
  ValidateRenderCommands(commands);
#endif

  for (auto& command : *commands) {
    auto shader_it = opengl->loaded_shaders.find(command.shader->uuid.value);
    ASSERT(shader_it != opengl->loaded_shaders.end());
    ShaderHandles& shader_handles = shader_it->second;

    glUseProgram(shader_handles.program_handle);

    SetConfigs(&command.config);
    SetCameraMatrices(opengl, command.camera);

    switch (command.type) {
      case RenderCommandType::kMesh:
        ExecuteMeshActions(opengl, &command, &shader_handles);
        break;
      case RenderCommandType::kNoop:
        continue;
      case RenderCommandType::kLast:
        NOT_REACHED() << "Invalid render command type.";
    }

    glUseProgram(NULL);
  }
}

}  // namespace

void OpenGLRendererBackend::ExecuteCommands(Renderer*,
                                            List<RenderCommand>* commands) {
  OpenGLExecuteCommands(this, commands);
}

// End Frame -------------------------------------------------------------------

void OpenGLRendererBackend::EndFrame(Renderer*) {
  // Return state to it's appropiate state.
  glDisable(GL_SCISSOR_TEST);
}

// Misc ------------------------------------------------------------------------

OpenGLRendererBackend::~OpenGLRendererBackend() {
  if (Valid(this))
    OpenGLShutdown(this);
}

}  // namespace opengl
}  // namespace warhol
