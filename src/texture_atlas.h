// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "src/texture.h"

#include "src/math/vec.h"

namespace warhol {

constexpr size_t kRock = 0;
constexpr size_t kDirt = 1;
constexpr size_t kGrassDirt = 2;
constexpr size_t kWook = 3;
constexpr size_t kConcreteBrick = 4;
constexpr size_t kConcrete = 5;
constexpr size_t kBrick = 6;
constexpr size_t kTnt = 7;
constexpr size_t kRedGridBroken = 8;
constexpr size_t kRedGrid = 9;
constexpr size_t kSpiderWeb = 10;
constexpr size_t kFlowerLightBlue = 11;
constexpr size_t kFlowerYellow = 12;
constexpr size_t kBlue = 13;
constexpr size_t kLittleTree = 14;

class TextureAtlas {
 public:
  // Represents the UVs coordinates of a particular entry in the atlas.
  struct UVs {
    Vec2<float> bottom_left;
    Vec2<float> top_right;
  };

  // x and y defines the amount of elements per row and columns.
  TextureAtlas(Texture atlas, size_t x, size_t y);

  UVs GetUVs(size_t index);


  // Amount of textures in the atlas.
  size_t count() const { return x_ * y_; }
  const Texture& texture() const { return tex_; }

  size_t x() const { return x_; }
  size_t y() const { return y_; }

 private:
  // Size of a particular texture.
  Vec2<size_t> TextureSize(size_t index) const;
  Vec2<float> TextureOffset(size_t index) const;
  Vec2<size_t> IndexToCoord(size_t index) const;

  Texture tex_;
  size_t x_, y_;
};

}  // namespace warhol
