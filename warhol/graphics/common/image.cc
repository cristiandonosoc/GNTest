// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/common/image.h"

#include <third_party/stb/stb_image.h>

#include "warhol/math/math.h"
#include "warhol/utils/assert.h"

namespace warhol {

Image Image::Create2DImageFromPath(const std::string& path) {
  Image image;
  image.type = Image::Type::k2D;
  image.format = Image::Format::kRGBA8;
  image.free_function = stbi_image_free;

  // We require 4 channels.
  image.data = stbi_load(path.data(),
                         &image.width, &image.height, &image.channels,
                         STBI_rgb_alpha);
  if (!image.valid())
    image.Release();

  image.data_size = image.width * image.height * STBI_rgb_alpha;
  image.mip_levels = (int)Math::floor(Math::log2(Math::max(image.width,
                                                           image.height)));
  // At minimum we need a 1.
  if (image.mip_levels < 1)
    image.mip_levels = 1;

  return image;
}

Image::Image() = default;
Image::~Image() {
  uint8_t* ptr = Release();
  ASSERT(free_function);
  if (ptr)
    free_function(ptr);
}

uint8_t* Image::Release() {
  uint8_t* tmp = data.value;
  data.clear();
  data_size = 0;
  width = -1;
  height = -1;
  channels = -1;
  return tmp;
}

}  // namespace warhol
