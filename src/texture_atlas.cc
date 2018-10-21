// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/texture_atlas.h"

#include <assert.h>

#include "src/utils/log.h"

namespace warhol {

TextureAtlas::TextureAtlas(Texture atlas, size_t x, size_t y)
    : tex_(std::move(atlas)), x_(x), y_(y) {}

TextureAtlas::UVs TextureAtlas::GetUVs(size_t index) {
  assert(index < count());

  UVs uvs = {};
  auto [x, y] = IndexToCoord(index);
  auto [offsetx, offsety] = TextureOffset(index);

  LOG(DEBUG) << "OFFSETX: " << offsetx << ", OFFSETY: " << offsety;

  uvs.bottom_left = { x * offsetx, y * offsety };
  uvs.top_right = { (x + 1) * offsetx, (y + 1) * offsety };
  return uvs;
}

// TODO(donosoc): Unhardcode from the uniform atlas.
Vec2<size_t> TextureAtlas::TextureSize(size_t) const {
  size_t x = texture().x() / this->x();
  size_t y = texture().y() / this->y();
  return {x, y};
}

Vec2<float>
TextureAtlas::TextureOffset(size_t index) const {
  auto[sizex, sizey] = TextureSize(index);
  return {(float)sizex / (float)texture().x(),
          (float)sizey / (float)texture().y()};
}

Vec2<size_t> TextureAtlas::IndexToCoord(size_t index) const {
  assert(index < count());
  return { index % x(), index / y() };
}

}  // namespace warhol
