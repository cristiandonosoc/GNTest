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
Handle<VkImageView> CreateImageView(Context*, VkImage, VkFormat,
                                    VkImageAspectFlags);

struct TransitionImageLayoutConfig {
  VkFormat format;
  VkImageLayout old_layout;
  VkImageLayout new_layout;
};
bool TransitionImageLayout(Context*, VkImage image,
                           const TransitionImageLayoutConfig&);

bool HasStencilComponent(VkFormat);

// Transforms warhol's internal formatting enums to the vulkan's equivalent.

VkFormat ToVulkan(Image::Format);
VkImageType ToVulkan(Image::Type);

}  // namespace vulkan
}  // namespace warhol
