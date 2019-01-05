// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "warhol/graphics/vulkan/def.h"

#include "warhol/graphics/vulkan/handle.h"

namespace warhol {
namespace vulkan {

struct Context;

struct BufferHandles {
  Handle<VkBuffer> buffer;
  Handle<VkDeviceMemory> memory;

  bool valid() const { return buffer.has_value() && memory.has_value(); }
};

bool CreateBuffer(Context* context, VkDeviceSize size, VkBufferUsageFlags usage,
                  VkMemoryPropertyFlags desired_properties,
                  BufferHandles* out);


// Copies one buffer and blocks waiting for the transfer.
bool CopyBuffer(Context* context, VkBuffer src_buffer, VkBuffer dst_buffer,
                VkDeviceSize size);

}  // namespace vulkan
}  // namespace warhol
