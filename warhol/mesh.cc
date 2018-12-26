// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/mesh.h"

#include <assert.h>

#include "warhol/graphics/GL/utils.h"

namespace warhol {

Mesh::Mesh() = default;

bool Mesh::Init() {
  assert(!initialized_);

  GL_CALL(glGenVertexArrays, 1, &vao_.handle);

  uint32_t buffers[2];
  GL_CALL(glGenBuffers, ARRAY_SIZE(buffers), buffers);
  vbo_.handle = buffers[0];
  ebo_.handle = buffers[1];

  initialized_ = true;
  return true;
}

void
Mesh::BufferData(std::vector<float> in_data,
                 std::vector<AttributeFormat> in_formats) {
  assert(initialized_);
  data = std::move(in_data);
  formats = std::move(in_formats);

  GL_CALL(glBindVertexArray, vao_.handle);
  GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, vbo_.handle);
  // TODO(Cristian): Pass in the storage type.
  GL_CALL(glBufferData, GL_ARRAY_BUFFER,
                        data.size() * sizeof(float), data.data(),
                        GL_STATIC_DRAW);

  for (const AttributeFormat& format : formats) {
    GL_CALL(glVertexAttribPointer, format.index,
                                   format.size,
                                   GL_FLOAT,
                                   GL_FALSE,  // Normalized.
                                   format.stride,
                                   (void*)(size_t)format.offset);
    GL_CALL(glEnableVertexAttribArray, format.index);
  }

  GL_CALL(glBindVertexArray, NULL);
  GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, NULL);
}

void Mesh::BufferIndices(std::vector<uint32_t> in_indices) {
  assert(initialized_);
  indices = std::move(in_indices);

  GL_CALL(glBindVertexArray, vao_.handle);
  GL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, ebo_.handle);
  GL_CALL(glBufferData, GL_ELEMENT_ARRAY_BUFFER,
                        indices.size() * sizeof(uint32_t), indices.data(),
                        GL_STATIC_DRAW);

  GL_CALL(glBindVertexArray, NULL);
  GL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, NULL);
}

void
Mesh::Bind() {
  assert(initialized_);
  GL_CALL(glBindVertexArray, vao_.handle);
}

}  // namespace warhol
