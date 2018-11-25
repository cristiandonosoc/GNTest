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

struct TextureMetadata {
  std::string path;

};

class TextureArray2D {
 public:
  TextureArray2D(std::string path, Pair<int> size);
  DELETE_COPY_AND_ASSIGN(TextureArray2D);

  bool Init();

  // -1 means invalid.
  int GetTextureIndex(int x, int y);

  const std::string& path() const { return path_; }
  const Pair<int>& size() const { return size_; }

 private:
  std::string path_;
  Pair<int> size_;

  GLHandle<GL_TEXTURE_2D_ARRAY> handle_;
  bool init_ = false;
};

}  // namespace warhol
