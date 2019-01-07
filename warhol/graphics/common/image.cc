// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/common/image.h"

#include <third_party/stb/stb_image.h>

namespace warhol {

Image Image::Create2DImageFromPath(const std::string& path) {
  Image image;
  image.type = Image::Type::k2D;
  image.format = Image::Format::kRGBA8;
  // We require 4 channels.
  image.data = stbi_load(path.data(),
                         &image.width, &image.height, nullptr,
                         STBI_rgb_alpha);
  if (!image.valid())
    image.Release();

  return image;
}

Image::Image() = default;
Image::~Image() {
  uint8_t* ptr = Release();
  if (ptr)
    stbi_image_free(ptr);
}

uint8_t* Image::Release() {
  uint8_t* tmp = data.value;
  data.clear();
  data_size = 0;
  width = -1;
  height = -1;
  return tmp;
}

}  // namespace warhol
