// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "warhol/graphics/common/image.h"
#include "warhol/graphics/vulkan/def.h"
#include "warhol/graphics/vulkan/handle.h"

namespace warhol {
namespace vulkan {

struct Context;
template <typename HandleType>
struct MemoryBacked;

struct CreateImageConfig {
  VkImageCreateInfo create_info;
  VkMemoryPropertyFlags properties;
};
MemoryBacked<VkImage>
CreateImage(Context*, const CreateImageConfig&);

// Returns VK_NULL_HANDLE
struct CreateImageViewConfig {
  VkImage image;
  VkFormat format;
  VkImageAspectFlags aspect_mask;
  uint32_t mip_levels = 1;
};
Handle<VkImageView> CreateImageView(Context*, const CreateImageViewConfig&);

struct TransitionImageLayoutConfig {
  VkFormat format;
  VkImageLayout old_layout;
  VkImageLayout new_layout;
  uint32_t mip_levels = 1;
};
bool TransitionImageLayout(Context*, VkImage image,
                           const TransitionImageLayoutConfig&);


struct GenerateMipmapsConfig {
  VkImage image;
  uint32_t width;
  uint32_t height;
  uint32_t mip_levels;
};
bool GenerateMipmaps(Context*, const GenerateMipmapsConfig&);

bool HasStencilComponent(VkFormat);

// Transforms warhol's internal formatting enums to the vulkan's equivalent.

VkFormat ToVulkan(Image::Format);
VkImageType ToVulkan(Image::Type);

}  // namespace vulkan
}  // namespace warhol
