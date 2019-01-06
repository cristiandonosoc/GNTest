// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "warhol/graphics/vulkan/def.h"

#include "warhol/graphics/vulkan/handle.h"

namespace warhol {
namespace vulkan {

struct Context;

struct DeviceBackedMemory {
  DeviceBackedMemory() = default;
  ~DeviceBackedMemory();
  DELETE_COPY_AND_ASSIGN(DeviceBackedMemory);
  DECLARE_MOVE_AND_ASSIGN(DeviceBackedMemory);

  bool host_visible() const { return data != nullptr; }

  Handle<VkBuffer> buffer;
  Handle<VkDeviceMemory> memory;

  VkDeviceSize size;
  // Not null if it's host visible.
  void* data = nullptr;
  VkDevice device = VK_NULL_HANDLE;
};

bool CreateBuffer(Context* context, VkDeviceSize size, VkBufferUsageFlags usage,
                  VkMemoryPropertyFlags desired_properties,
                  DeviceBackedMemory* out);


// Copies one buffer and blocks waiting for the transfer.
bool CopyBuffer(Context* context, VkBuffer src_buffer, VkBuffer dst_buffer,
                VkDeviceSize size);

}  // namespace vulkan
}  // namespace warhol
