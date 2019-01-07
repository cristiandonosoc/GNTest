// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/vulkan/image_utils.h"

#include "warhol/graphics/vulkan/commands.h"
#include "warhol/graphics/vulkan/context.h"
#include "warhol/graphics/vulkan/handle.h"
#include "warhol/utils/assert.h"

namespace warhol {
namespace vulkan {

bool TransitionImageLayout(Context* context, VkImage image,
                           const TransitionImageLayoutConfig& config) {
  Handle<VkCommandBuffer> command_buffer = BeginSingleTimeCommands(context);
  if (!command_buffer.has_value())
    return false;

  VkImageMemoryBarrier barrier = {};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = config.old_layout;
  barrier.newLayout = config.new_layout;

  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

  barrier.image = image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  barrier.srcAccessMask = 0;  // TODO
  barrier.dstAccessMask = 0;  // TODO





  if (!EndSingleTimeCommands(context, *command_buffer))
    return false;

  return true;
}

// Warhol -> Vulkan ------------------------------------------------------------

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
