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

  if (!VK_CALL(vkBindImageMemory, *context->device, *image_handle,
                                  *memory_handle, 0)) {
    return {};
  }

  MemoryBacked<VkImage> backed_image;
  backed_image.handle = std::move(image_handle);
  backed_image.memory = std::move(memory_handle);

  return backed_image;
}

Handle<VkImageView>
CreateImageView(Context* context, const CreateImageViewConfig& config) {
  VkImageViewCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  create_info.image = config.image;
  create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  create_info.format = config.format;
  create_info.subresourceRange.aspectMask = config.aspect_mask;
  create_info.subresourceRange.baseMipLevel = 0;
  create_info.subresourceRange.levelCount = config.mip_levels;
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
GetTransitionMasks(VkImageLayout old_layout, VkImageLayout new_layout,
                   TransitionMasks* out) {
  if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED) {
    switch (new_layout) {
      case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        out->src_access_mask = 0;
        out->dst_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT;
        out->src_stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        out->dst_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        return true;
      case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        out->src_access_mask = 0;
        out->dst_access_mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        out->src_stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        out->dst_stage_mask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        return true;
      default:
        break;
    }
  } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    switch (new_layout) {
      case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        out->src_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT;
        out->dst_access_mask = VK_ACCESS_SHADER_READ_BIT;
        out->src_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        out->dst_stage_mask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        return true;
      default:
        break;
    }
  }

  LOG(ERROR) << "Could not get transition mask " << EnumToString(old_layout)
             << " -> " << EnumToString(new_layout);
  return false;
}

VkImageAspectFlags
GetImageAspectFlags(VkFormat format, VkImageLayout new_layout) {
  VkImageAspectFlags aspect_flags = 0;

  // If this is a depth mask, we say so. We also check if there is a stencil
  // component.
  if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (HasStencilComponent(format))
      aspect_flags |= VK_IMAGE_ASPECT_STENCIL_BIT;
  } else {
    aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT;
  }

  return aspect_flags;
}

}  // namespace

bool TransitionImageLayout(Context* context, VkImage image,
                           const TransitionImageLayoutConfig& config) {
  Handle<VkCommandBuffer> command_buffer = BeginSingleTimeCommands(context);
  if (!command_buffer.has_value())
    return false;

  TransitionMasks transition_masks;
  if (!GetTransitionMasks(config.old_layout, config.new_layout,
                          &transition_masks)) {
    return false;
  }

  VkImageMemoryBarrier barrier = {};
  barrier.image = image;
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = config.old_layout;
  barrier.newLayout = config.new_layout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

  auto aspect_flags = GetImageAspectFlags(config.format, config.new_layout);
  barrier.subresourceRange.aspectMask = aspect_flags;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = config.mip_levels;
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

// GenerateMipmaps -------------------------------------------------------------

namespace {

inline int GetDstMip(int mip) {
  if (mip > 1)
    return mip / 2;
  return mip;
}

bool CheckForFilteringSupport(VkPhysicalDevice device, VkFormat format) {
  VkFormatProperties format_properties;
  vkGetPhysicalDeviceFormatProperties(device, format, &format_properties);

  return format_properties.optimalTilingFeatures &
         VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
}

}  // namespace

bool GenerateMipmaps(Context* context, const GenerateMipmapsConfig& config) {
  if (!CheckForFilteringSupport(context->physical_device, config.format)) {
    LOG(ERROR) << "No linear filtering support.";
    return false;
  }

  Handle<VkCommandBuffer> command_buffer = BeginSingleTimeCommands(context);
  if (!command_buffer.has_value())
    return false;

  VkImageMemoryBarrier barrier = {};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.image = config.image;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  barrier.subresourceRange.levelCount = 1;

  int mip_width = config.width;
  int mip_height = config.height;

  for (uint32_t i = 1; i < config.mip_levels; i++) {
    LOG(DEBUG) << "Mip level " << i - 1 << ": Transition to src.";

    // We prepare each mip map for destination, marking the current one to be
    // a src transfer, while keeping the next one as dst.
    barrier.subresourceRange.baseMipLevel = i - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

    vkCmdPipelineBarrier(*command_buffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier);

    LOG(DEBUG) << "Mip level " << i << ": Blitting.";

    // We blit the new mip map.
    VkImageBlit blit = {};
    blit.srcOffsets[0] = {0, 0, 0};
    blit.srcOffsets[1] = {mip_width, mip_height, 1};
    blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.srcSubresource.mipLevel = i - 1;
    blit.srcSubresource.layerCount = 1;
    blit.srcSubresource.baseArrayLayer = 0;
    blit.dstOffsets[0] = {0, 0, 0};
    blit.dstOffsets[1] = {GetDstMip(mip_width), GetDstMip(mip_height), 1};
    blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.dstSubresource.mipLevel = i;
    blit.dstSubresource.layerCount = 1;
    blit.dstSubresource.baseArrayLayer = 0;
    vkCmdBlitImage(*command_buffer,
                   config.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   config.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   1, &blit,
                   VK_FILTER_LINEAR);

    LOG(DEBUG) << "Mip level " << i - 1 << ": transition to shared read.";

    // We barrier the src mipmap to read access from the shader.
    barrier.subresourceRange.baseMipLevel = i - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(*command_buffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier);

    mip_width = GetDstMip(mip_width);
    mip_height = GetDstMip(mip_height);
  }

  LOG(DEBUG) << "Mip level " << config.mip_levels - 1 << ": transition to shared read.";

  // We need to transfer the last mip level to shader read.
  barrier.subresourceRange.baseMipLevel = config.mip_levels - 1;
  barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

  vkCmdPipelineBarrier(*command_buffer,
                       VK_PIPELINE_STAGE_TRANSFER_BIT,
                       VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                       0,
                       0, nullptr,
                       0, nullptr,
                       1, &barrier);

  if (!EndSingleTimeCommands(context, *command_buffer))
    return false;
  return true;
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
