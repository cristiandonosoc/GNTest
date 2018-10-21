// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "src/texture.h"

#include "src/math/vec.h"

namespace warhol {

#define INDEX(row, column) 16 * row + column

constexpr size_t kGravel = INDEX(15, 0);
constexpr size_t kRock = INDEX(15, 1);
constexpr size_t kDirt = INDEX(15, 2);
constexpr size_t kGrassDirt = INDEX(15, 3);
constexpr size_t kWood = INDEX(15, 4);
constexpr size_t kConcreteBrick = INDEX(15, 5);
constexpr size_t kConcrete = INDEX(15, 6);
constexpr size_t kBrick = INDEX(15, 7);
constexpr size_t kTnt = INDEX(15, 8);
constexpr size_t kRedGridBroken = INDEX(15, 9);
constexpr size_t kRedGrid = INDEX(15, 10);
constexpr size_t kSpiderWeb = INDEX(15, 11);
constexpr size_t kFlowerLightBlue = INDEX(15, 12);
constexpr size_t kFlowerYellow = INDEX(15, 13);
constexpr size_t kBlue = INDEX(15, 14);
constexpr size_t kLittleTree = INDEX(15, 15);

constexpr size_t kGrass = INDEX(3, 12);

constexpr size_t kCake = INDEX(7, 12);

constexpr size_t kTransparent = INDEX(4, 5);

constexpr size_t kCrack0 = INDEX(0, 0);
constexpr size_t kCrack1 = INDEX(0, 1);
constexpr size_t kCrack2 = INDEX(0, 2);
constexpr size_t kCrack3 = INDEX(0, 3);
constexpr size_t kCrack4 = INDEX(0, 4);
constexpr size_t kCrack5 = INDEX(0, 5);
constexpr size_t kCrack6 = INDEX(0, 6);
constexpr size_t kCrack7 = INDEX(0, 7);
constexpr size_t kCrack8 = INDEX(0, 8);
constexpr size_t kCrack9 = INDEX(0, 9);

class TextureAtlas {
 public:
  // Represents the UVs coordinates of a particular entry in the atlas.
  struct UVs {
    Vec2<float> bottom_left;
    Vec2<float> top_right;
  };

  // x and y defines the amount of elements per row and columns.
  TextureAtlas(Texture atlas, size_t x, size_t y);

  UVs GetUVs(size_t index) const;

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
