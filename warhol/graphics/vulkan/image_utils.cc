// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/vulkan/image_utils.h"

#include "warhol/graphics/vulkan/commands.h"
#include "warhol/graphics/vulkan/context.h"
#include "warhol/graphics/vulkan/handle.h"
#include "warhol/utils/assert.h"

namespace warhol {
namespace vulkan {

namespace {

struct TransitionMasks {
  VkAccessFlags src_access_mask;
  VkAccessFlags dst_access_mask;
  VkPipelineStageFlags src_stage_mask;
  VkPipelineStageFlags dst_stage_mask;
};

bool
DetermineTransitionMasks(VkImageLayout old_layout, VkImageLayout new_layout,
                         TransitionMasks* out) {
  if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
      new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    out->src_access_mask = 0;
    out->dst_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT;
    out->src_stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    out->dst_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    out-> src_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT;
    out-> dst_access_mask = VK_ACCESS_SHADER_READ_BIT;

    out->src_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    out->dst_stage_mask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else {
    return false;
  }

  return true;
}

}  // namespace

bool TransitionImageLayout(Context* context, VkImage image,
                           const TransitionImageLayoutConfig& config) {
  Handle<VkCommandBuffer> command_buffer = BeginSingleTimeCommands(context);
  if (!command_buffer.has_value())
    return false;

  TransitionMasks transition_masks;
  if (!DetermineTransitionMasks(
          config.old_layout, config.new_layout, &transition_masks)) {
    return false;
  }

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

  barrier.srcAccessMask = transition_masks.src_access_mask;
  barrier.dstAccessMask = transition_masks.dst_access_mask;

  vkCmdPipelineBarrier(*command_buffer,
                       transition_masks.src_stage_mask,
                       transition_masks.dst_stage_mask,
                       0,
                       0, nullptr,      // Memory barriers.
                       0, nullptr,      // Buffer memory barriers.
                       1, &barrier);    // Image memory barriers.

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
