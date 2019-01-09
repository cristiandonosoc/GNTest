// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "warhol/graphics/common/image.h"
#include "warhol/graphics/vulkan/def.h"
#include "warhol/graphics/vulkan/handle.h"

namespace warhol {
namespace vulkan {

struct Context;

// Returns VK_NULL_HANDLE
Handle<VkImageView> CreateImageView(Context*, VkImage, VkFormat);

struct TransitionImageLayoutConfig {
  VkFormat format;
  VkImageLayout old_layout;
  VkImageLayout new_layout;
};
bool TransitionImageLayout(Context*, VkImage image,
                           const TransitionImageLayoutConfig&);

// Transforms warhol's internal formatting enums to the vulkan's equivalent.

VkFormat ToVulkan(Image::Format);
VkImageType ToVulkan(Image::Type);



}  // namespace vulkan
}  // namespace warhol
