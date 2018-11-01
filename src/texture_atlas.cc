// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/texture_atlas.h"

#include <assert.h>

#include "src/utils/log.h"

namespace warhol {

TextureAtlas::TextureAtlas(Texture atlas, size_t x, size_t y)
    : tex_(std::move(atlas)), x_(x), y_(y) {}

Pair<Pair<float>> TextureAtlas::GetUVs(size_t index) const {
  assert(index < count());

  Pair<Pair<float>> uvs = {};
  auto [x, y] = IndexToCoord(index);
  auto [offsetx, offsety] = TextureOffset(index);

  uvs.min() = { x * offsetx, y * offsety };
  uvs.max() = { (x + 1) * offsetx, (y + 1) * offsety };
  return uvs;
}

// TODO(donosoc): Unhardcode from the uniform atlas.
Pair<size_t> TextureAtlas::TextureSize(size_t) const {
  size_t x = texture().x() / this->x();
  size_t y = texture().y() / this->y();
  return {x, y};
}

Pair<float>
TextureAtlas::TextureOffset(size_t index) const {
  auto[sizex, sizey] = TextureSize(index);
  return {(float)sizex / (float)texture().x(),
          (float)sizey / (float)texture().y()};
}

Pair<size_t> TextureAtlas::IndexToCoord(size_t index) const {
  assert(index < count());
  return { index % x(), index / y() };
}

}  // namespace warhol
