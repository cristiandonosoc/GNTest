// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "warhol/graphics/vulkan/def.h"
#include "warhol/graphics/vulkan/memory_utils.h"

namespace warhol {
namespace vulkan {

struct StagingBuffer {
  uint8_t* data() const { return buffer.allocation.data; }

  bool submitted = false;
  Handle<VkCommandBuffer> command_buffer = {};  // Not owning.
  MemoryBacked<VkBuffer> buffer = {};
  Handle<VkFence> fence = {};
};

// StagingManager is double buffered. This is so that if we haven't flushed
// before the buffer is filled, we switch to the other and flush the first one.
struct StagingManager {
  // If max is hit, we switch buffers and flush.
  int current_buffer = 0;
  VkDeviceMemory memory;          // TODO(Cristian): Owning?
  Handle<VkCommandPool> command_pool = {};

  VkDeviceSize buffer_size = 0;
  StagingBuffer buffers[Definitions::kNumFrames];
  uint8_t* mapped_data = nullptr;
};

StagingManager* GetSingletonStagingBuffer();
bool Init(Context*, StagingManager*);
void Shutdown(StagingManager*);
uint8_t* Stage(VkDeviceSize size, VkDeviceSize alignment,
               VkCommandBuffer* command_buffer, VkBuffer* buffer,
               VkDeviceSize* buffer_offset);

void Flush(StagingManager*);
// Wait until the command buffer with the copy commands is done processing.
void Wait(StagingManager*);

}  // namespace vulkan
}  // namespace warhol
