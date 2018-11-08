// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <vector>

#include "src/utils/clear_on_move.h"
#include "src/utils/macros.h"

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
   ~Mesh();
  DELETE_COPY_AND_ASSIGN(Mesh);

  bool Init();
  // IMPORTANT: Both data and formats must be given when calling these.
  void BindData();
  void BindFormats();

  ClearOnMove<uint32_t> vao;
  ClearOnMove<uint32_t> vbo;
  ClearOnMove<uint32_t> ebo;

  std::vector<AttributeFormat> formats;
  std::vector<float> data;
  std::vector<uint32_t> indices;
  bool initialized_ = false;
};

}  // namespace warhol
