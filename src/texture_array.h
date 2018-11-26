// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <string>

#include "src/graphics/GL/utils.h"
#include "src/math/vec.h"
#include "src/utils/clear_on_move.h"
#include "src/utils/macros.h"

namespace warhol {

class TextureArray2D {
 public:
  // |stride|: How many elements into the "matrix" of elements.
  //           0 means a flat array.
  // |size|: X, Y means width, height.
  //         Z means how many elements there will be in the array.
  TextureArray2D(Pair3<int> size, int stride, GLenum format);
  DELETE_COPY_AND_ASSIGN(TextureArray2D);

  bool Init();
  // |size| must match size_.x * size_.y
  bool AddElement(uint8_t* data, int size);

  // -1 means invalid.
  int GetTextureIndex(int x, int y);

  const Pair3<int>& size() const { return size_; }
  int stride() const { return stride_; }

 private:
  Pair3<int> size_;
  int stride_ = 0;
  int element_count_ = 0;

  GLHandle<GL_TEXTURE_2D_ARRAY> handle_;
  GLenum format_;
  bool init_ = false;
};

}  // namespace warhol
