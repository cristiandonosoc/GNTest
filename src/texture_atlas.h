// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "src/texture.h"

#include "src/math/vec.h"

namespace warhol {

class TextureAtlas {
 public:
  // x and y defines the amount of elements per row and columns.
  TextureAtlas(Texture atlas, size_t x, size_t y);

  Pair<Pair<float>> GetUVs(size_t index) const;

  // Amount of textures in the atlas.
  size_t count() const { return x_ * y_; }
  const Texture& texture() const { return tex_; }

  size_t x() const { return x_; }
  size_t y() const { return y_; }

 private:
  // Size of a particular texture.
  Pair<size_t> TextureSize(size_t index) const;
  Pair<float> TextureOffset(size_t index) const;
  Pair<size_t> IndexToCoord(size_t index) const;

  Texture tex_;
  size_t x_, y_;
};

}  // namespace warhol
