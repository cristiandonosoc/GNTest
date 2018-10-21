// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "src/texture.h"

namespace warhol {

class TextureAtlas {
 public:
  // Represents the UVs coordinates of a particular entry in the atlas.
  struct UVs {
    // Top-Left, Top-Right, Bottom-Left, Bottom-Right.
    float tl, tr, bl, br;
  };

  // x and y defines the amount of elements per row and columns.
  TextureAtlas(Texture atlas, int x, int y);

  UVs GetUVs(size_t index);

  const Texture& texture() const { return tex_; }

 private:
  Texture tex_;
  int x_, y_;
};

}  // namespace warhol
