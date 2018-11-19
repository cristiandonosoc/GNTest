// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "src/texture.h"

#include "src/math/vec.h"

namespace warhol {

#define INDEX(row, column) 16 * row + column

// TODO(Cristian): Move these definitions out of here.
struct VoxelType {
  static constexpr uint8_t kGravel = INDEX(15, 0);
  static constexpr uint8_t kRock = INDEX(15, 1);
  static constexpr uint8_t kDirt = INDEX(15, 2);
  static constexpr uint8_t kGrassDirt = INDEX(15, 3);
  static constexpr uint8_t kWood = INDEX(15, 4);
  static constexpr uint8_t kConcreteBrick = INDEX(15, 5);
  static constexpr uint8_t kConcrete = INDEX(15, 6);
  static constexpr uint8_t kBrick = INDEX(15, 7);
  static constexpr uint8_t kTnt = INDEX(15, 8);
  static constexpr uint8_t kRedGridBroken = INDEX(15, 9);
  static constexpr uint8_t kRedGrid = INDEX(15, 10);
  static constexpr uint8_t kSpiderWeb = INDEX(15, 11);
  static constexpr uint8_t kFlowerLightBlue = INDEX(15, 12);
  static constexpr uint8_t kFlowerYellow = INDEX(15, 13);
  static constexpr uint8_t kBlue = INDEX(15, 14);
  static constexpr uint8_t kLittleTree = INDEX(15, 15);

  static constexpr uint8_t kGrass = INDEX(3, 12);

  static constexpr uint8_t kCake = INDEX(7, 12);

  static constexpr uint8_t kTransparent = INDEX(4, 5);

  static constexpr uint8_t kCrack0 = INDEX(0, 0);
  static constexpr uint8_t kCrack1 = INDEX(0, 1);
  static constexpr uint8_t kCrack2 = INDEX(0, 2);
  static constexpr uint8_t kCrack3 = INDEX(0, 3);
  static constexpr uint8_t kCrack4 = INDEX(0, 4);
  static constexpr uint8_t kCrack5 = INDEX(0, 5);
  static constexpr uint8_t kCrack6 = INDEX(0, 6);
  static constexpr uint8_t kCrack7 = INDEX(0, 7);
  static constexpr uint8_t kCrack8 = INDEX(0, 8);
  static constexpr uint8_t kCrack9 = INDEX(0, 9);
};

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
