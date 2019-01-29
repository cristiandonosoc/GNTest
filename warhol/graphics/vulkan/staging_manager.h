// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "warhol/graphics/vulkan/def.h"
#include "warhol/graphics/vulkan/memory_utils.h"
#include "warhol/utils/log.h"

namespace warhol {
namespace vulkan {

struct StagingBuffer {
  DEFAULT_CONSTRUCTOR(StagingBuffer);
  DEFAULT_MOVE_AND_ASSIGN(StagingBuffer);
  ~StagingBuffer();
  uint8_t* data() const { return buffer.allocation.data; }

  Handle<VkCommandBuffer> command_buffer = {};  // Not owning.
  MemoryBacked<VkBuffer> buffer = {};
  Handle<VkFence> fence = {};

  VkDeviceSize offset = 0;
  bool submitting = false;
};

// StagingManager is double buffered. This is so that if we haven't flushed
// before the buffer is filled, we switch to the other and flush the first one.
struct StagingManager {
  DEFAULT_CONSTRUCTOR(StagingManager);
  ~StagingManager();
  DEFAULT_MOVE_AND_ASSIGN(StagingManager);

  // If max is hit, we switch buffers and flush.
  Handle<VkCommandPool> command_pool = {};
  VkDeviceSize buffer_size = 0;

  int current_buffer = 0;
  StagingBuffer buffers[Definitions::kNumFrames];
};

void InitStagingManager(Context*, StagingManager*, VkDeviceSize buffer_size);

struct StageToken {
  bool valid() const;

  VkCommandBuffer command_buffer = VK_NULL_HANDLE;
  VkBuffer buffer = VK_NULL_HANDLE;
  VkDeviceSize size = 0;
  VkDeviceSize offset  = 0;
  uint8_t* data = nullptr;
};

StageToken Stage(Context*, StagingManager*, VkDeviceSize size,
                 VkDeviceSize alignment);

// NOTE: Can switch the |current_buffer| index.
void Flush(Context*, StagingManager*);

}  // namespace vulkan
}  // namespace warhol
