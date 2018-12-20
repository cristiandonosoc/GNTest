// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <vector>

#include "src/utils/clear_on_move.h"
#include "src/utils/macros.h"

#include "src/graphics/GL/utils.h"

namespace warhol {

// TODO(Cristian): Do we want to diminish the size of this?
struct AttributeFormat {
  uint32_t index;
  int32_t size;
  uint32_t stride;
  uint32_t offset;
};

// Mesh represents a set of data associated with buffers in the graphics.
// A mesh has no idea where it is. The transformation matrices must be set
// before rendering.
class Mesh {
 public:
  Mesh();
  DELETE_COPY_AND_ASSIGN(Mesh);

  bool Init();
  // IMPORTANT: Both data and formats must be given when calling these.
  void BufferData(std::vector<float> data, std::vector<AttributeFormat>);
  void BufferIndices(std::vector<uint32_t>);
  void Bind();

  std::vector<AttributeFormat> formats;
  std::vector<float> data;
  std::vector<uint32_t> indices;

  bool initialized() const { return initialized_; }

 private:
  bool initialized_ = false;
  GLHandle<GL_VERTEX_ARRAY> vao_;
  GLHandle<GL_ARRAY_BUFFER> vbo_;
  GLHandle<GL_ELEMENT_ARRAY_BUFFER> ebo_;
};

}  // namespace warhol
