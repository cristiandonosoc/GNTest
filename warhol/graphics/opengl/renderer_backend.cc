// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/opengl/renderer_backend.h"

#include "warhol/scene/camera.h"
#include "warhol/graphics/common/mesh.h"
#include "warhol/graphics/common/shader.h"
#include "warhol/graphics/common/texture.h"
#include "warhol/graphics/renderer.h"
#include "warhol/utils/log.h"

namespace warhol {
namespace opengl {

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

}  // namespace

// Init ------------------------------------------------------------------------

bool OpenGLInit(OpenGLRendererBackend* opengl) {
  int res = gl3wInit();
  if (res != GL3W_OK) {
    LOG(WARNING) << "Got non-OK GL3W result: " << Gl3wInitResultToString(res);
    return false;
  }

  GL_CALL(glGenBuffers, 1, &opengl->camera_ubo);
  GL_CALL(glBindBuffer, GL_UNIFORM_BUFFER, opengl->camera_ubo);
  GL_CALL(glBufferData, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
  GL_CALL(glBindBuffer, GL_UNIFORM_BUFFER, NULL);

  GL_CALL(glBindBufferBase, GL_UNIFORM_BUFFER, 0, opengl->camera_ubo);

  opengl->loaded = true;
  return true;
}

// Stage Shader ----------------------------------------------------------------

namespace {

bool CompileShader(Shader* shader, const char* source, GLenum shader_kind,
                   uint32_t* out_handle) {
  uint32_t handle = glCreateShader(shader_kind);
  if (!handle) {
    LOG(ERROR) << "Shader " << shader->name << ": Could not allocate shader.";
    return false;
  }

  // Compile the shader source.
  const GLchar* gl_src = source;
  GL_CALL(glShaderSource, handle, 1, &gl_src, 0);
  GL_CALL(glCompileShader, handle);

  GLint success = 0;
  GL_CALL(glGetShaderiv, handle, GL_COMPILE_STATUS, &success);
  if (success == GL_FALSE) {
    GLchar log[2048];
    GL_CALL(glGetShaderInfoLog, handle, sizeof(log), 0, log);
    GL_CALL(glDeleteShader, handle);
    LOG(ERROR) << "Shader " << shader->name << ": Error compiling "
               << GLEnumToString(shader_kind) << " shader: " << log;
    return false;
  }

  *out_handle = false;
  return true;
}

bool CompileVertShader(Shader* shader, uint32_t* out_handle) {
  return CompileShader(shader, shader->vert_source.c_str(), GL_VERTEX_SHADER,
                       out_handle);
}

bool CompileFragShader(Shader* shader, uint32_t* out_handle) {
  return CompileShader(shader, shader->vert_source.c_str(), GL_FRAGMENT_SHADER,
                       out_handle);
}

}  // namespace

bool OpenGLStageShader(OpenGLRendererBackend* opengl, Shader* shader) {
  auto it = opengl->loaded_shaders.find(shader->uuid);
  if (it != opengl->loaded_shaders.end()) {
    LOG(ERROR) << "Shader " << shader->name << " is already loaded.";
    return false;
  }

  // Compile the shaders and and program.
  uint32_t vert_handle = 0;
  uint32_t frag_handle = 0;
  if (!CompileVertShader(shader, &vert_handle) ||
      !CompileFragShader(shader, &frag_handle)) {
    return false;
  }

  uint32_t prog_handle = glCreateProgram();
  if (prog_handle == 0) {
    LOG(ERROR) << "glCreateProgram: could not allocate a program";
    return false;
  }

  // Link 'em.
  GL_CALL(glAttachShader, prog_handle, vert_handle);
  GL_CALL(glAttachShader, prog_handle, frag_handle);
  GL_CALL(glLinkProgram, prog_handle);
  glDeleteShader(vert_handle);
  glDeleteShader(frag_handle);

  GLint success = 0;
  GL_CALL(glGetProgramiv, prog_handle, GL_LINK_STATUS, &success);
  if (success == GL_FALSE) {
    GLchar log[2048];
    GL_CALL(glGetProgramInfoLog, prog_handle, sizeof(log), 0, log);
    LOG(ERROR) << "Could not link shader: " << log;
    return false;
  }

  // Get the uniform buffer blocks.
  const char* block_name = "Camera";
  uint32_t block_index = glGetUniformBlockIndex(prog_handle, block_name);
  if (block_index == GL_INVALID_INDEX) {
    LOG(ERROR) << "Could not find uniform buffer block " << block_name;
    glDeleteProgram(prog_handle);
    return false;
  }

  GL_CALL(glUniformBlockBinding, prog_handle, block_index, 0);
  opengl->loaded_shaders[shader->uuid] = prog_handle;
  return true;
}

// Unstage Shader --------------------------------------------------------------

void OpenGLUnstageShader(OpenGLRendererBackend* opengl, Shader* shader) {
  auto it = opengl->loaded_shaders.find(shader->uuid);
  ASSERT(it != opengl->loaded_shaders.end());

  GL_CALL(glDeleteProgram, it->second);
  opengl->loaded_shaders.erase(it);
}

// Stage Mesh ------------------------------------------------------------------

namespace {

MeshHandles GenerateMeshHandles() {
  uint32_t buffers[2];
  GL_CALL(glGenBuffers, ARRAY_SIZE(buffers), buffers);

  uint32_t vao;
  GL_CALL(glGenVertexArrays, 1, &vao);

  MeshHandles handles;
  handles.vbo = buffers[0];
  handles.ebo = buffers[1];
  handles.vao = vao;
  return handles;
}

void DeleteMeshHandles(MeshHandles* handles) {
  GL_CALL(glDeleteBuffers, 2, (GLuint*)handles);
  GL_CALL(glDeleteVertexArrays, 1, handles->vao);
}

void BindMeshHandles(MeshHandles* handles) {
  GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, handles->vbo);
  GL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, handles->ebo);
  GL_CALL(glBindVertexArray, handles->vao);
}

void UnbindMeshHandles() {
  GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, NULL);
  GL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, NULL);
  GL_CALL(glBindVertexArray, NULL);
}

void BufferVertices(Mesh* mesh) {
  GL_CALL(glBufferData, GL_ARRAY_BUFFER, VerticesSize(mesh),
          mesh->vertices.data(), GL_STATIC_DRAW);

  // Pos.
  GL_CALL(glVertexAttribPointer, 0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
  GL_CALL(glEnableVertexAttribArray, 0);
  // Color.
  GL_CALL(glVertexAttribPointer, 1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
          offsetof(Vertex, color));
  GL_CALL(glEnableVertexAttribArray, 1);
  // UV.
  GL_CALL(glVertexAttribPointer, 2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
          offsetof(Vertex, uv));
  GL_CALL(glEnableVertexAttribArray, 2);
}

void BufferIndices(Mesh* mesh) {
  GL_CALL(glBufferData, GL_ELEMENT_ARRAY_BUFFER, IndicesSize(mesh),
          mesh->indices.data(), GL_STATIC_DRAW);
}

}  // namespace

bool OpenGLStageMesh(OpenGLRendererBackend* opengl, Mesh* mesh) {
  auto it = opengl->loaded_meshes.find(mesh->uuid);
  if (it != opengl->loaded_meshes.end()) {
    LOG(ERROR) << "Reloading mesh " << mesh->name;
    return false;
  }

  MeshHandles handles = GenerateMeshHandles();
  BindMeshHandles(&handles);

  BufferVertices(mesh);
  BufferIndices(mesh);

  UnbindMeshHandles();

  return true;
}

// Unstage Mesh ----------------------------------------------------------------

void OpenGLUnstageMesh(OpenGLRendererBackend* opengl, Mesh* mesh) {
  auto it = opengl->loaded_meshes.find(mesh->uuid);
  ASSERT(it != opengl->loaded_meshes.end());

  DeleteMeshHandles(&it->second);
  opengl->loaded_meshes.erase(it);
}

// Stage Texture ---------------------------------------------------------------

bool OpenGLStageTexture(OpenGLRendererBackend* opengl, Texture* texture) {
  auto it = opengl->loaded_textures.find(texture->uuid);
  if (it != opengl->loaded_textures.end()) {
    LOG(ERROR) << "Shader " << texture->name << " is already loaded.";
    return false;
  }

  uint32_t handle;
  GL_CALL(glGenTextures, 1, handle);
  GL_CALL(glBindTexture, GL_TEXTURE_2D, handle);

  // Setup wrapping/filtering options.
  GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Send the bits over.
  // TODO(Cristian): Check for errors.
  // TODO(Cristian): When multi-threading is done, use this async.
  //                 Think what to do about in memory buffer.
  GL_CALL(glTexImage2D, GL_TEXTURE_2D,          // target
                        0,                      // level
                        GL_RGBA,                // internalformat
                        texture->x,             // width,
                        texture->y,             // height
                        0,                      // border
                        GL_RGBA,                // format
                        GL_UNSIGNED_BYTE,       // type,
                        texture->data.value);
  GL_CALL(glGenerateMipmap, GL_TEXTURE_2D);

  opengl->loaded_textures[texture->uuid] = handle;
  return true;
}

// Unstage Texture -------------------------------------------------------------

void OpenGLUnstageTexture(OpenGLRendererBackend* opengl, Texture* texture) {
  auto it = opengl->loaded_textures.find(texture->uuid);
  ASSERT(it != opengl->loaded_textures.end());

  GL_CALL(glDeleteTextures, 1, it->second);
  opengl->loaded_textures.erase(it);
}

// Start Frame -----------------------------------------------------------------

void OpenGLStartFrame(Renderer* renderer) {
  GL_CALL(glClearColor, renderer->clear_color.x,
                        renderer->clear_color.y,
                        renderer->clear_color.z);
  GL_CALL(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// Execute Commands ------------------------------------------------------------

namespace {

void SetCameraMatrices(OpenGLRendererBackend* opengl, Camera* camera) {
  if (opengl->last_set_camera == camera)
    return;

  void* data = &camera->projection;
  GL_CALL(glBindBuffer, GL_UNIFORM_BUFFER, opengl->camera_ubo);
  GL_CALL(glBufferSubData, GL_UNIFORM_BUFFER, 0, 2 * sizeof(glm::mat4), data);
  GL_CALL(glBindBuffer, GL_UNIFORM_BUFFER, NULL);

  opengl->last_set_camera = camera;
}

void ExecuteMeshActions(OpenGLRendererBackend* opengl,
                        LinkedList<MeshRenderAction>* mesh_actions) {
  for (MeshRenderAction& action : *mesh_actions) {
    auto it = opengl->loaded_meshes.find(action.mesh->uuid);
    ASSERT(it != opengl->loaded_meshes.end());

    MeshHandles& handles = it->second;
    GL_CALL(glBindVertexArray, handles.vao);
    GL_CALL(glDrawElements, GL_TRIANGLES, action.mesh->indices.size() / 3,
                            GL_UNSIGNED_INT, NULL);
  }
  GL_CALL(glBindVertexArray, NULL);
}

}  // namespace

void OpenGLExecuteCommands(OpenGLRendererBackend* opengl,
                           LinkedList<RenderCommand>* commands) {
  for (auto& command : *commands) {
    auto shader_it = opengl->loaded_shaders.find(command.shader->uuid);
    ASSERT(shader_it != opengl->loaded_shaders.end());

    GL_CALL(glUseProgram, shader_it->second);

    SetCameraMatrices(opengl, command.camera);

    switch (command.type) {
      case RenderCommandType::kMesh:
        ExecuteMeshActions(opengl, command.mesh_actions);
        break;
      case RenderCommandType::kLast:
        NOT_REACHED("Invalid render command type.");
    }

    GL_CALL(glUseProgram, NULL);
  }
}

// Shutdown --------------------------------------------------------------------

void OpenGLShutdown(OpenGLRendererBackend* opengl) {
  ASSERT(Valid(opengl));

  for (auto& [shader_uuid, handle] : opengl->loaded_shaders) {
    GL_CALL(glDeleteProgram, handle);
  }

  for (auto& [mesh_uuid, handles] : opengl->loaded_meshes) {
    DeleteMeshHandles(&handles);
  }

  if (opengl->camera_ubo != 0) {
    GL_CALL(glDeleteBuffers, 1, &opengl->camera_ubo);
    opengl->camera_ubo = 0;
  }

  opengl->loaded = false;
}

// Virtual Interface "Dispatch" ------------------------------------------------

bool OpenGLRendererBackend::Init(Renderer*) {
  return OpenGLInit(this);
}

void OpenGLRendererBackend::Shutdown() {
  return OpenGLShutdown(this);
}

bool OpenGLRendererBackend::StageShader(Shader* shader) {
  return OpenGLStageShader(this, shader);
}

void OpenGLRendererBackend::UnstageShader(Shader* shader) {
  OpenGLUnstageShader(this, shader);
}

bool OpenGLRendererBackend::StageMesh(Mesh* mesh) {
  return OpenGLStageMesh(this, mesh);
}

void OpenGLRendererBackend::UnstageMesh(Mesh* mesh) {
  OpenGLUnstageMesh(this, mesh);
}

bool OpenGLRendererBackend::StageTexture(Texture* texture) {
  return OpenGLStageTexture(this, texture);
}

void OpenGLRendererBackend::UnstageTexture(Texture* texture) {
  OpenGLUnstageTexture(this, texture);
}

void OpenGLRendererBackend::StartFrame(Renderer* renderer) {
  OpenGLStartFrame(renderer);
}

void OpenGLRendererBackend::ExecuteCommands(
    Renderer*, LinkedList<RenderCommand>* commands) {
  OpenGLExecuteCommands(this, commands);
}

void OpenGLRendererBackend::EndFrame(Renderer*) {
  // No op.
}

}  // namespace opengl
}  // namespace warhol
