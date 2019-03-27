// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/opengl/renderer_backend.h"

#include "warhol/scene/camera.h"
#include "warhol/graphics/common/mesh.h"
#include "warhol/graphics/common/shader.h"
#include "warhol/graphics/common/texture.h"
#include "warhol/graphics/common/renderer.h"
#include "warhol/utils/log.h"

namespace warhol {
namespace opengl {

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

}  // namespace

bool OpenGLInit(OpenGLRendererBackend* opengl) {
  int res = gl3wInit();
  if (res != GL3W_OK) {
    LOG(WARNING) << "Got non-OK GL3W result: " << Gl3wInitResultToString(res);
    return false;
  }

  uint32_t camera_ubo;
  GL_CHECK(glGenBuffers(1, &camera_ubo));
  opengl->camera_ubo = camera_ubo;

  GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, opengl->camera_ubo.value));
  GL_CHECK(glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL,
                        GL_STATIC_DRAW));
  GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, NULL));

  GL_CHECK(glBindBufferBase(GL_UNIFORM_BUFFER, 0, opengl->camera_ubo.value));

  opengl->loaded = true;
  return true;
}

bool OpenGLRendererBackend::Init(Renderer*, Window*) {
  return OpenGLInit(this);
}

// Shutdown --------------------------------------------------------------------

namespace {

void DeleteMeshHandles(MeshHandles* handles);   // Forward declaration.

void OpenGLShutdown(OpenGLRendererBackend* opengl) {
  ASSERT(Valid(opengl));

  for (auto& [shader_uuid, handle] : opengl->loaded_shaders) {
    GL_CHECK(glDeleteProgram(handle));
  }
  opengl->loaded_shaders.clear();

  for (auto& [mesh_uuid, handles] : opengl->loaded_meshes) {
    DeleteMeshHandles(&handles);
  }
  opengl->loaded_meshes.clear();

  if (opengl->camera_ubo.has_value()) {
    GL_CHECK(glDeleteBuffers(1, &opengl->camera_ubo.value));
    opengl->camera_ubo = 0;
  }
  opengl->camera_ubo.clear();

  opengl->loaded = false;
}

}  // namespace

void OpenGLRendererBackend::Shutdown() {
  return OpenGLShutdown(this);
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
  GL_CHECK(glShaderSource(handle, 1, &gl_src, 0));
  GL_CHECK(glCompileShader(handle));

  GLint success = 0;
  GL_CHECK(glGetShaderiv(handle, GL_COMPILE_STATUS, &success));
  if (success == GL_FALSE) {
    GLchar log[2048];
    GL_CHECK(glGetShaderInfoLog(handle, sizeof(log), 0, log));
    GL_CHECK(glDeleteShader(handle));
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
  GL_CHECK(glAttachShader(prog_handle, vert_handle));
  GL_CHECK(glAttachShader(prog_handle, frag_handle));
  GL_CHECK(glLinkProgram(prog_handle));
  glDeleteShader(vert_handle);
  glDeleteShader(frag_handle);

  GLint success = 0;
  GL_CHECK(glGetProgramiv(prog_handle, GL_LINK_STATUS, &success));
  if (success == GL_FALSE) {
    GLchar log[2048];
    GL_CHECK(glGetProgramInfoLog(prog_handle, sizeof(log), 0, log));
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

  GL_CHECK(glUniformBlockBinding(prog_handle, block_index, 0));
  opengl->loaded_shaders[shader->uuid] = prog_handle;
  return true;
}

bool OpenGLRendererBackend::StageShader(Shader* shader) {
  return OpenGLStageShader(this, shader);
}

// Unstage Shader --------------------------------------------------------------

void OpenGLUnstageShader(OpenGLRendererBackend* opengl, Shader* shader) {
  auto it = opengl->loaded_shaders.find(shader->uuid);
  ASSERT(it != opengl->loaded_shaders.end());

  GL_CHECK(glDeleteProgram(it->second));
  opengl->loaded_shaders.erase(it);
}

void OpenGLRendererBackend::UnstageShader(Shader* shader) {
  OpenGLUnstageShader(this, shader);
}

// Stage Mesh ------------------------------------------------------------------

namespace {

MeshHandles GenerateMeshHandles() {
  uint32_t buffers[2];
  GL_CHECK(glGenBuffers(ARRAY_SIZE(buffers), buffers));

  uint32_t vao;
  GL_CHECK(glGenVertexArrays(1, &vao));

  MeshHandles handles;
  handles.vbo = buffers[0];
  handles.ebo = buffers[1];
  handles.vao = vao;
  return handles;
}

void DeleteMeshHandles(MeshHandles* handles) {
  GL_CHECK(glDeleteBuffers(2, (GLuint*)handles));
  GL_CHECK(glDeleteVertexArrays(1, &handles->vao));
}

void BindMeshHandles(MeshHandles* handles) {
  GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, handles->vbo));
  GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handles->ebo));
  GL_CHECK(glBindVertexArray(handles->vao));
}

void UnbindMeshHandles() {
  GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, NULL));
  GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL));
  GL_CHECK(glBindVertexArray(NULL));
}

void BufferVertices(Mesh* mesh) {
  GL_CHECK(glBufferData(GL_ARRAY_BUFFER, VerticesSize(mesh),
          mesh->vertices.data(), GL_STATIC_DRAW));

  // Pos.
  GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0));
  GL_CHECK(glEnableVertexAttribArray(0));
  // Color.
  GL_CHECK(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
          (void*)offsetof(Vertex, color)));
  GL_CHECK(glEnableVertexAttribArray(1));
  // UV.
  GL_CHECK(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
          (void*)offsetof(Vertex, uv)));
  GL_CHECK(glEnableVertexAttribArray(2));
}

void BufferIndices(Mesh* mesh) {
  GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndicesSize(mesh),
          mesh->indices.data(), GL_STATIC_DRAW));
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

bool OpenGLRendererBackend::StageMesh(Mesh* mesh) {
  return OpenGLStageMesh(this, mesh);
}

// Unstage Mesh ----------------------------------------------------------------

void OpenGLUnstageMesh(OpenGLRendererBackend* opengl, Mesh* mesh) {
  auto it = opengl->loaded_meshes.find(mesh->uuid);
  ASSERT(it != opengl->loaded_meshes.end());

  DeleteMeshHandles(&it->second);
  opengl->loaded_meshes.erase(it);
}

void OpenGLRendererBackend::UnstageMesh(Mesh* mesh) {
  OpenGLUnstageMesh(this, mesh);
}

// Stage Texture ---------------------------------------------------------------

namespace {

bool OpenGLStageTexture(OpenGLRendererBackend* opengl, Texture* texture) {
  auto it = opengl->loaded_textures.find(texture->uuid);
  if (it != opengl->loaded_textures.end()) {
    LOG(ERROR) << "Shader " << texture->name << " is already loaded.";
    return false;
  }

  uint32_t handle;
  GL_CHECK(glGenTextures(1, &handle));
  GL_CHECK(glBindTexture(GL_TEXTURE_2D, handle));

  // Setup wrapping/filtering options.
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

  // Send the bits over.
  // TODO(Cristian): Check for errors.
  // TODO(Cristian): When multi-threading is done, use this async.
  //                 Think what to do about in memory buffer.
  GL_CHECK(glTexImage2D(GL_TEXTURE_2D,          // target
                        0,                      // level
                        GL_RGBA,                // internalformat
                        texture->x,             // width,
                        texture->y,             // height
                        0,                      // border
                        GL_RGBA,                // format
                        GL_UNSIGNED_BYTE,       // type,
                        texture->data.value));
  GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D));

  opengl->loaded_textures[texture->uuid] = handle;
  return true;
}

}  // namespace

bool OpenGLRendererBackend::StageTexture(Texture* texture) {
  return OpenGLStageTexture(this, texture);
}

// Unstage Texture -------------------------------------------------------------

namespace {

void OpenGLUnstageTexture(OpenGLRendererBackend* opengl, Texture* texture) {
  auto it = opengl->loaded_textures.find(texture->uuid);
  ASSERT(it != opengl->loaded_textures.end());

  GL_CHECK(glDeleteTextures(1, &it->second));
  opengl->loaded_textures.erase(it);
}

}  // namespace

void OpenGLRendererBackend::UnstageTexture(Texture* texture) {
  OpenGLUnstageTexture(this, texture);
}

// Start Frame -----------------------------------------------------------------

namespace {

void OpenGLStartFrame(Renderer* renderer) {
  GL_CHECK(glClearColor(renderer->clear_color.x,
                        renderer->clear_color.y,
                        renderer->clear_color.z,
                        1.0f));
  GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
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
  GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, opengl->camera_ubo.value));
  GL_CHECK(glBufferSubData(GL_UNIFORM_BUFFER, 0, 2 * sizeof(glm::mat4), data));
  GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, NULL));

  opengl->last_set_camera = camera;
}

void ExecuteMeshActions(OpenGLRendererBackend* opengl,
                        LinkedList<MeshRenderAction>* mesh_actions) {
  for (MeshRenderAction& action : *mesh_actions) {
    auto it = opengl->loaded_meshes.find(action.mesh->uuid);
    ASSERT(it != opengl->loaded_meshes.end());

    MeshHandles& handles = it->second;
    GL_CHECK(glBindVertexArray(handles.vao));
    GL_CHECK(glDrawElements(GL_TRIANGLES, action.mesh->indices.size() / 3,
                            GL_UNSIGNED_INT, NULL));
  }
  GL_CHECK(glBindVertexArray(NULL));
}

void OpenGLExecuteCommands(OpenGLRendererBackend* opengl,
                           LinkedList<RenderCommand>* commands) {
  for (auto& command : *commands) {
    auto shader_it = opengl->loaded_shaders.find(command.shader->uuid);
    ASSERT(shader_it != opengl->loaded_shaders.end());

    GL_CHECK(glUseProgram(shader_it->second));

    SetCameraMatrices(opengl, command.camera);

    switch (command.type) {
      case RenderCommandType::kMesh:
        ExecuteMeshActions(opengl, command.mesh_actions);
        break;
      case RenderCommandType::kLast:
        NOT_REACHED("Invalid render command type.");
    }

    GL_CHECK(glUseProgram(NULL));
  }
}

}  // namespace

void OpenGLRendererBackend::ExecuteCommands(
    Renderer*, LinkedList<RenderCommand>* commands) {
  OpenGLExecuteCommands(this, commands);
}

// End Frame -------------------------------------------------------------------

void OpenGLRendererBackend::EndFrame(Renderer*) {
  // No op.
}

// Misc ------------------------------------------------------------------------

OpenGLRendererBackend::~OpenGLRendererBackend() {
  if (Valid(this))
    OpenGLShutdown(this);
}

}  // namespace opengl
}  // namespace warhol
