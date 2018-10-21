// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/texture_atlas.h"

namespace warhol {

TextureAtlas::TextureAtlas(Texture atlas, int x, int y) : tex_(std::move(atlas)), x_(x), y_(y) {

}

}  // namespace warhol
