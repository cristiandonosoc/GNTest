// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/vulkan/image_utils.h"

#include "warhol/utils/assert.h"

namespace warhol {
namespace vulkan {

VkFormat ToVulkan(Image::Format format) {
  switch (format) {
    case Image::Format::kRGBA8: return VK_FORMAT_B8G8R8A8_UNORM;
    case Image::Format::kLast: break;
  }

  NOT_REACHED();
  return VK_FORMAT_UNDEFINED;
}

VkImageType ToVulkan(Image::Type type) {
  switch (type) {
    case Image::Type::k1D: return VK_IMAGE_TYPE_1D;
    case Image::Type::k2D: return VK_IMAGE_TYPE_2D;
    case Image::Type::k3D: return VK_IMAGE_TYPE_3D;
    case Image::Type::kLast: break;
  }

  NOT_REACHED();
  return (VkImageType)0;
}

}  // namespace vulkan
}  // namespace warhol
