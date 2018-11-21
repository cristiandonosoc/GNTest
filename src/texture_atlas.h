// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "src/texture.h"

#include "src/math/vec.h"

namespace warhol {

#define INDEX(row, column) 16 * row + column

// TODO(Cristian): Move these definitions out of here.
enum class VoxelType : uint8_t {
  kGravel = INDEX(15, 0),
  kRock = INDEX(15, 1),
  kDirt = INDEX(15, 2),
  kGrassDirt = INDEX(15, 3),
  kWood = INDEX(15, 4),
  kConcreteBrick = INDEX(15, 5),
  kConcrete = INDEX(15, 6),
  kBrick = INDEX(15, 7),
  kTnt = INDEX(15, 8),
  kRedGridBroken = INDEX(15, 9),
  kRedGrid = INDEX(15, 10),
  kSpiderWeb = INDEX(15, 11),
  kFlowerLightBlue = INDEX(15, 12),
  kFlowerYellow = INDEX(15, 13),
  kBlue = INDEX(15, 14),
  kLittleTree = INDEX(15, 15),

  kGrass = INDEX(3, 12),

  kCake = INDEX(7, 12),

  kTransparent = INDEX(4, 5),

  kCrack0 = INDEX(0, 0),
  kCrack1 = INDEX(0, 1),
  kCrack2 = INDEX(0, 2),
  kCrack3 = INDEX(0, 3),
  kCrack4 = INDEX(0, 4),
  kCrack5 = INDEX(0, 5),
  kCrack6 = INDEX(0, 6),
  kCrack7 = INDEX(0, 7),
  kCrack8 = INDEX(0, 8),
  kCrack9 = INDEX(0, 9),

  kNone = 0xFF,
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
