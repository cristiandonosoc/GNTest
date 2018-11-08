// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/mesh.h"

#include <assert.h>

#include "src/graphics/GL/utils.h"

namespace warhol {

bool Mesh::Init() {
  assert(!initialized_);

  glGenVertexArrays(1, &vao.value);

  uint32_t buffers[2];
  glGenBuffers(ARRAY_SIZE(buffers), buffers);
  vbo.value = buffers[0];
  ebo.value = buffers[1];

  if (CHECK_GL_ERRORS(__PRETTY_FUNCTION__))
    exit(1);

  initialized_ = true;
  return true;
}

Mesh::~Mesh() {
  if (vao.value) {
    glDeleteVertexArrays(1, &vao.value);
    vao.clear();
  }
  if (vbo.value) {
    glDeleteBuffers(1, &vbo.value);
    vbo.clear();
  }
  if (ebo.value) {
    glDeleteBuffers(1, &ebo.value);
    ebo.clear();
  }

  if (CHECK_GL_ERRORS(__PRETTY_FUNCTION__))
    exit(1);
}

void Mesh::BindData() {
  assert(initialized_);
  assert(!data.empty());
  // Do the GL instantiation.
  glBindVertexArray(vao.value);

  glBindBuffer(GL_ARRAY_BUFFER, vbo.value);
  // TODO(Cristian): Pass in the storage type.
  glBufferData(GL_ARRAY_BUFFER, data.size(), data.data(), GL_STATIC_DRAW);

  if (CHECK_GL_ERRORS(__PRETTY_FUNCTION__))
    exit(1);

  BindFormats();
}

void Mesh::BindFormats() {
  assert(initialized_);
  assert(!formats.empty());

  glBindVertexArray(vao.value);

  // Only one data array per mesh.
  glBindBuffer(GL_ARRAY_BUFFER, vbo.value);

  for (const AttributeFormat& format : formats) {
    glVertexAttribPointer(format.index,
                          format.size,
                          GL_FLOAT,     // TODO(Cristian): Abstract data type?
                          GL_FALSE,     // Normalized.
                          format.stride,
                          (void*)(size_t)format.offset);
    glEnableVertexAttribArray(format.index);
  }
}

}  // namespace warhol
