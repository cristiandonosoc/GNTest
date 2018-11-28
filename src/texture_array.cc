// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/texture_array.h"

#include <assert.h>

#include "src/utils/log.h"

namespace warhol {

TextureArray2D::TextureArray2D(Pair3<int> size, int stride, GLenum format)
    : size_(std::move(size)), stride_(stride), format_(format) {}

bool TextureArray2D::Init() {
  GL_CALL(glGenTextures, 1, &*handle_);
  GL_CALL(glBindTexture, GL_TEXTURE_2D_ARRAY, *handle_);

  // Allocate the texture space.
  LOG(DEBUG) << __PRETTY_FUNCTION__ << ": Format is " << GLEnumToString(format_);
  GL_CALL(glTexImage3D, GL_TEXTURE_2D_ARRAY,
                        0,  // No mip-map for now.
                        format_,
                        size_.x, size_.y, size_.z,
                        0,  // No border
                        format_,
                        GL_UNSIGNED_BYTE,
                        (void*)NULL);  // No data, we are just allocating space.

  init_ = true;
  return true;
}

bool TextureArray2D::AddElement(uint8_t* data, int size) {
  if (element_count_ >= size_.y) {
    LOG(WARNING) << "Element count exceeds capacity (max: " << size_.y << ").";
    return false;
  }

  if (size != size_.x * size_.y) {
    LOG(WARNING) << "Expected different size sub-image (Expected: "
                 << size_.x * size_.y << ", got: " << size << ").";
    return false;
  }

  GL_CALL(glBindTexture, GL_TEXTURE_2D_ARRAY, *handle_);
  GL_CALL(glTexSubImage3D, GL_TEXTURE_2D_ARRAY,
                           0,  // Mip-map level.
                           0, 0,  element_count_,    // X, Y, Z coords of the sub image.
                           size_.x, size_.y, 1,      // X, Y, Z size of sub image.
                           format_,
                           GL_UNSIGNED_BYTE,
                           data);

  return true;
}

}  // namespace warhol
