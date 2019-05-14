// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "warhol/graphics/common/image.h"
#include "warhol/graphics/vulkan/def.h"
#include "warhol/graphics/vulkan/handle.h"
#include "warhol/graphics/vulkan/memory_utils.h"

namespace warhol {
namespace vulkan {

struct Context;

struct AllocImageConfig {
  VkImageCreateInfo create_info;
  MemoryUsage memory_usage = MemoryUsage::kNone;
};
MemoryBacked<VkImage>
AllocImage(Context*, AllocImageConfig*);

// Returns VK_NULL_HANDLE
struct CreateImageViewConfig {
  VkImage image;
  VkFormat format;
  VkImageAspectFlags aspect_mask;
  uint32_t mip_levels = 1;
};
Handle<VkImageView> CreateImageView(Context*, CreateImageViewConfig*);

struct TransitionImageLayoutConfig {
  VkFormat format;
  VkImageLayout old_layout;
  VkImageLayout new_layout;
  uint32_t mip_levels = 1;
};
bool TransitionImageLayout(Context*, VkImage, TransitionImageLayoutConfig*);

// CreateImage: AllocImage + CreateImageView + TransitionImageLayout.

// NOTE: Will override |view_config.image| with the newly created VkImage.
struct CreateImageConfig {
  AllocImageConfig alloc_config = {};
  CreateImageViewConfig view_config = {};
  TransitionImageLayoutConfig transition_config = {};
};
struct CreateImageResult {
  bool valid() const { return image.has_value() && image_view.has_value(); }

  MemoryBacked<VkImage> image = {};
  Handle<VkImageView> image_view = {};
};
CreateImageResult CreateImage(Context*, CreateImageConfig*);

struct GenerateMipmapsConfig {
  VkImage image;
  VkFormat format;
  uint32_t width;
  uint32_t height;
  uint32_t mip_levels;
};
bool GenerateMipmaps(Context*, GenerateMipmapsConfig*);

bool CopyBufferToImage(Context*, const Image&, VkBuffer src, VkImage dst);

bool HasStencilComponent(VkFormat);

// Transforms warhol's internal formatting enums to the vulkan's equivalent.

VkFormat ToVulkan(Image::Format);
VkImageType ToVulkan(Image::Type);

}  // namespace vulkan
}  // namespace warhol
