// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <vector>

namespace warhol {

class Plane {
 public:
  // Will return a plane that's centered in the origin with width and length
  // provided. Will also have UV coordinates equal to the plane position.
  static std::vector<float> Create(float width, float length);


};

}  // namespace warhol
