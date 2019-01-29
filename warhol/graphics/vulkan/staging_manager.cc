// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/vulkan/staging_manager.h"

#include "warhol/graphics/vulkan/commands.h"
#include "warhol/graphics/vulkan/context.h"
#include "warhol/graphics/vulkan/utils.h"
#include "warhol/utils/align.h"
#include "warhol/utils/macros.h"

namespace warhol {
namespace vulkan {

// StagingBuffer ---------------------------------------------------------------

namespace {

void InitStagingBuffer(Context* context, StagingManager* manager,
                       StagingBuffer* out) {
    // Memory back the buffer.
    AllocBufferConfig alloc_config = {};
    alloc_config.size = manager->buffer_size;
    alloc_config.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    alloc_config.memory_usage = MemoryUsage::kCPUToGPU;
    auto buffer = AllocBuffer(context, &alloc_config);
    ASSERT(buffer.has_value());

    // Create the command buffer.
    auto command_buffer = CreateCommandBuffer(context, *manager->command_pool);
    ASSERT(command_buffer.has_value());
    LOG(DEBUG) << "Extra handle: " << command_buffer.extra_handle();

    // Add a begin pass to the command buffer.
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VK_CHECK(vkBeginCommandBuffer, *command_buffer, &begin_info);

    // Create the fence.
    VkFence fence_out;
    VkFenceCreateInfo fence_create_info = {};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    VK_CHECK(vkCreateFence, *context->device, &fence_create_info, nullptr,
                            &fence_out);
    Handle<VkFence> fence(context, fence_out);

    out->buffer = std::move(buffer);
    out->command_buffer = std::move(command_buffer);
    out->fence = std::move(fence);
}

void WaitOnStage(Context* context, StagingBuffer* stage) {
  if (stage->submitting == false)
    return;

  VK_CHECK(vkWaitForFences, *context->device, 1, &stage->fence.value(),
                            VK_TRUE, UINT64_MAX);
  VK_CHECK(vkResetFences, *context->device, 1, &stage->fence.value());

  stage->offset = 0;
  stage->submitting = false;

  VkCommandBufferBeginInfo begin_info = {};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  VK_CHECK(vkBeginCommandBuffer, *stage->command_buffer, &begin_info);
}

void Clear(StagingBuffer* stage) {
  stage->fence.Clear();
  stage->buffer.handle.Clear();
  stage->command_buffer.Clear();

  stage->offset = 0;
  stage->submitting = false;
}

}  // namespace

StagingBuffer::~StagingBuffer() = default;

// StatingManager --------------------------------------------------------------

namespace {

inline StagingBuffer* CurrentStage(StagingManager* manager) {
  return manager->buffers + manager->current_buffer;
}

}  // namespace

StagingManager::~StagingManager() {
  for (size_t i = 0; i < ARRAY_SIZE(buffers); i++) {
    Clear(&buffers[i]);
  }
}

void InitStagingManager(Context* context, StagingManager* manager,
                        VkDeviceSize buffer_size) {
  manager->buffer_size = buffer_size;

  LOG(DEBUG) << "Creating Staging Manager Command Pool.";

  // This command pool is to be used over and over.
  VkCommandPool command_pool_out;
  VkCommandPoolCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  create_info.queueFamilyIndex =
      context->physical_device_info.queue_family_indices.graphics;
  VK_CHECK(vkCreateCommandPool, *context->device, &create_info, nullptr,
                                &command_pool_out);
  manager->command_pool.Set(context, command_pool_out);

  LOG(DEBUG) << "Initializing Staging Buffers.";

  // Allocate the buffer and associate the values.
  for (int i = 0; i < ARRAY_SIZE(manager->buffers); i++) {
    InitStagingBuffer(context, manager, &manager->buffers[i]);
  }
}

bool StageToken::valid() const {
  return command_buffer != VK_NULL_HANDLE &&
         buffer != VK_NULL_HANDLE &&
         offset != 0;
}


StageToken
Stage(Context* context, StagingManager* manager, VkDeviceSize size,
      VkDeviceSize alignment) {
  if (size > manager->buffer_size) {
    LOG(ERROR) << "Cannot allocate " << BytesToString(size)
               << " in GPU transfer buffer (Capacity: "
               << BytesToString(manager->buffer_size) << ").";
    return {};
  }

  StagingBuffer* stage = CurrentStage(manager);

  // If the current staging buffer doesn't have enough space, we flush it.
  VkDeviceSize aligned_offset = Align(stage->offset, alignment);
  if ((aligned_offset + size) >= manager->buffer_size && !stage->submitting)
    Flush(context, manager);

  // IMPORTANT: Flush could have switched the |current_buffer| index!
  stage = CurrentStage(manager);
  if (stage->submitting)
    WaitOnStage(context, stage);

  // At this point we know that this StagingBuffer is empty, so we don't need
  // to align.
  stage->offset += size;

  StageToken token = {};
  token.command_buffer = *stage->command_buffer;
  token.buffer = *stage->buffer.handle;
  token.size = size;
  token.offset = 0;
  token.data = stage->data();

  return token;
}

void Flush(Context* context, StagingManager* manager) {
  StagingBuffer* stage = CurrentStage(manager);
  if (stage->submitting || stage->offset == 0)
    return;

  VkMemoryBarrier barrier = {};
  barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
  barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT |
                          VK_ACCESS_INDEX_READ_BIT;
  vkCmdPipelineBarrier(*stage->command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                       VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0,
                       1, &barrier,
                       0, nullptr,
                       0, nullptr);

  vkEndCommandBuffer(*stage->command_buffer);

  VkMappedMemoryRange memory_range = {};
  memory_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
  memory_range.memory = *stage->buffer.allocation.memory;
  memory_range.size = (VkDeviceSize)VK_WHOLE_SIZE;

  VK_CHECK(vkFlushMappedMemoryRanges, *context->device, 1, &memory_range);

  VkSubmitInfo submit_info = {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &stage->command_buffer.value();

  VK_CHECK(vkQueueSubmit, context->graphics_queue, 1, &submit_info,
                          *stage->fence);
  stage->submitting = true;

  manager->current_buffer++;
  manager->current_buffer %= ARRAY_SIZE(manager->buffers);
}
}  // namespace vulkan
}  // namespace warhol
