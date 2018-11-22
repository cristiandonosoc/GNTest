// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/texture_array.h"

#include <third_party/stb/stb_image.h>

namespace warhol {

TextureArray2D::TextureArray2D(std::string path, Pair<int> size)
    : path_(std::move(path)), size_(std::move(size)) {}

bool TextureArray2D::Init() {
  stbi_set_flip_vertically_on_load(true);




  init_ = true;
  return true;
}

}  // namespace warhol
