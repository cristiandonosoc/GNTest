// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/vulkan/image_utils.h"

#include "warhol/graphics/vulkan/commands.h"
#include "warhol/graphics/vulkan/context.h"
#include "warhol/graphics/vulkan/memory.h"
#include "warhol/graphics/vulkan/utils.h"
#include "warhol/utils/assert.h"

namespace warhol {
namespace vulkan {

// Image Creation --------------------------------------------------------------

MemoryBacked<VkImage> CreateImage(Context* context,
                                  const CreateImageConfig& config) {
  VkImage image;
  if (!VK_CALL(vkCreateImage, *context->device, &config.create_info, nullptr,
                              &image)) {
    return {};
  }
  Handle<VkImage> image_handle(context, image);

  VkMemoryRequirements memory_reqs;
  vkGetImageMemoryRequirements(*context->device, image, &memory_reqs);

  auto memory_handle = AllocMemory(context, memory_reqs, config.properties);
  if (!memory_handle.has_value())
    return {};

  MemoryBacked<VkImage> backed_image;
  backed_image.handle = std::move(image_handle);
  backed_image.memory = std::move(memory_handle);

  return backed_image;
}

Handle<VkImageView>
CreateImageView(Context* context, VkImage image, VkFormat format,
                VkImageAspectFlags aspect_flags) {
  VkImageViewCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  create_info.image = image;
  create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  create_info.format = format;
  create_info.subresourceRange.aspectMask = aspect_flags;
  create_info.subresourceRange.baseMipLevel = 0;
  create_info.subresourceRange.levelCount = 1;
  create_info.subresourceRange.baseArrayLayer = 0;
  create_info.subresourceRange.layerCount = 1;

  VkImageView image_view;
  if (!VK_CALL(vkCreateImageView, *context->device, &create_info, nullptr,
                                  &image_view)) {
    return {};
  }
  return {context, image_view};
}

// TransitionImageLayout -------------------------------------------------------

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

bool HasStencilComponent(VkFormat format) {
  return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
         format == VK_FORMAT_D24_UNORM_S8_UINT;
}

// Warhol -> Vulkan ------------------------------------------------------------

VkFormat ToVulkan(Image::Format format) {
  switch (format) {
    case Image::Format::kRGBA8: return VK_FORMAT_R8G8B8A8_UNORM;
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
