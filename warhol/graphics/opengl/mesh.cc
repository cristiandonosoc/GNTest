// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/opengl/mesh.h"

#include "warhol/graphics/common/mesh.h"
#include "warhol/graphics/opengl/renderer_backend.h"

namespace warhol {
namespace opengl {

void DeleteMeshHandles(MeshHandles* handles) {
  glDeleteBuffers(2, (GLuint*)handles);
  glDeleteVertexArrays(1, &handles->vao);
}

// Stage Mesh ------------------------------------------------------------------

namespace {

MeshHandles GenerateMeshHandles() {
  uint32_t buffers[2];
  glGenBuffers(ARRAY_SIZE(buffers), buffers);

  uint32_t vao;
  glGenVertexArrays(1, &vao);

  MeshHandles handles;
  handles.vbo = buffers[0];
  handles.ebo = buffers[1];
  handles.vao = vao;
  return handles;
}

void UnbindMeshHandles() {
  // Always unbind the VAO first, so that it doesn't overwrite.
  glBindVertexArray(NULL);
  glBindBuffer(GL_ARRAY_BUFFER, NULL);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);
}

GLenum AttributeTypeToGL(AttributeType type) {
  switch (type) {
    case AttributeType::kFloat:
      return GL_FLOAT;
    case AttributeType::kUint8:
      return GL_UNSIGNED_BYTE;
    case AttributeType::kLast:
      break;
  }

  NOT_REACHED();
  return GL_NONE;
}

void BindAttributes(Mesh* mesh) {
  ASSERT(!mesh->attributes.empty());
  GLsizei stride = 0;
  for (auto& attribute : mesh->attributes) {
    ASSERT(attribute.type != AttributeType::kLast);
    stride += (GLsizei)GetSize(&attribute);
  }

  // The shader layout must coincide with these attribute orders.
  int location = 0;
  GLsizei offset = 0;
  for (auto& attribute : mesh->attributes) {
    glVertexAttribPointer(location,
                          attribute.count,
                          AttributeTypeToGL(attribute.type),
                          attribute.normalized ? GL_TRUE : GL_FALSE,
                          stride,
                          (GLvoid*)(intptr_t)offset);
    glEnableVertexAttribArray(location);
    offset += (GLsizei)GetSize(&attribute);
    location++;
  }
}

void BufferVertices(Mesh* mesh, MeshHandles* handles) {
  glBindBuffer(GL_ARRAY_BUFFER, handles->vbo);
  glBufferData(GL_ARRAY_BUFFER, mesh->vertices.size, Data(&mesh->vertices),
               GL_STATIC_DRAW);

  BindAttributes(mesh);
}

void BufferIndices(Mesh* mesh, MeshHandles* handles) {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handles->ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               mesh->indices.size,
               Data(&mesh->indices),
               GL_STATIC_DRAW);
}

}  // namespace

bool OpenGLStageMesh(OpenGLRendererBackend* opengl, Mesh* mesh) {
  uint64_t uuid = mesh->uuid.value;
  LOG(DEBUG) << "Staging mesh " << mesh->name << " (uuid: " << uuid<< ").";
  auto it = opengl->loaded_meshes.find(uuid);
  if (it != opengl->loaded_meshes.end()) {
    LOG(ERROR) << "Reloading mesh " << mesh->name;
    return false;
  }

  // Always bind the VAO first, so that it doesn't overwrite.
  MeshHandles handles = GenerateMeshHandles();

  glBindVertexArray(handles.vao);

  BufferVertices(mesh, &handles);
  BufferIndices(mesh, &handles);

  UnbindMeshHandles();

  opengl->loaded_meshes[uuid] = std::move(handles);
  mesh->staged = true;

  return true;
}

// RendererUploadMeshRange -----------------------------------------------------

bool OpenGLRendererUploadMeshRange(OpenGLRendererBackend* opengl,
                                   Mesh* mesh,
                                   IndexRange vertex_range,
                                   IndexRange index_range) {
  uint64_t uuid = mesh->uuid.value;
  auto it = opengl->loaded_meshes.find(uuid);
  if (it == opengl->loaded_meshes.end()) {
    LOG(ERROR) << "Uploading range on non-staged mesh " << mesh->name;
    return false;
  }

  SCOPE_LOCATION() << "Mesh UUID: " << mesh->uuid.value;

  MeshHandles& handles = it->second;

  // Vertices.
  {
    uint32_t size = GetSize(vertex_range);
    if (size == 0)
      size = Used(&mesh->vertices);
    uint32_t offset = GetOffset(vertex_range);

    glBindBuffer(GL_ARRAY_BUFFER, handles.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, Data(&mesh->vertices));
    glBindBuffer(GL_ARRAY_BUFFER, NULL);
  }

  // Indices.
  {
    uint32_t size = GetSize(index_range);
    if (size == 0)
      size = Used(&mesh->indices);
    uint32_t offset = GetOffset(index_range);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handles.ebo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, size,
                    Data(&mesh->indices));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);
  }

  return true;
}

// Unstage Mesh ----------------------------------------------------------------

void OpenGLUnstageMesh(OpenGLRendererBackend* opengl, Mesh* mesh) {
  uint64_t uuid = mesh->uuid.value;
  LOG(DEBUG) << "Unstaging mesh " << mesh->name << " (uuid: " << uuid<< ").";
  auto it = opengl->loaded_meshes.find(uuid);
  ASSERT(it != opengl->loaded_meshes.end());

  DeleteMeshHandles(&it->second);
  opengl->loaded_meshes.erase(it);
  mesh->staged = false;
}

}  // namespace opengl
}  // namespace warhol
