// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <utility>

#include "warhol/graphics/vulkan/def.h"
#include "warhol/graphics/vulkan/handle.h"
#include "warhol/utils/assert.h"

namespace warhol {

struct Image;

namespace vulkan {

struct Context;

// Definition at the end of the header.
template <typename HandleType>
struct MemoryBacked;

// Memory ----------------------------------------------------------------------

Handle<VkDeviceMemory>
AllocMemory(Context* context, const VkMemoryRequirements& memory_reqs,
            const VkMemoryPropertyFlags& desired_properties);

// VkBuffer --------------------------------------------------------------------

// Creates a VkBuffer and allocates it.
struct AllocBufferConfig {
  VkDeviceSize size;
  VkBufferUsageFlags usage;
  VkMemoryPropertyFlags properties;
};
bool AllocBuffer(Context*, const AllocBufferConfig&,
                 MemoryBacked<VkBuffer>* out);

// Copies one buffer and blocks waiting for the transfer.
bool CopyBuffer(Context* context, VkBuffer src_buffer, VkBuffer dst_buffer,
                VkDeviceSize size);

// VkImage ---------------------------------------------------------------------

bool CopyBufferToImage(Context*, const Image&, VkBuffer src, VkImage dst);

// MemoryBacked Handles --------------------------------------------------------
//
// Represents a handle (VkBuffer, VkImage, etc.) that is backed by some memory
// memory in the GPU (VkDeviceMemory). This struct holds the memory as a
// resource and will destroy it on destruction.

template <typename HandleType>
struct MemoryBacked {
  // Constructors
  MemoryBacked() = default;
  ~MemoryBacked() {
    if (data == nullptr)
      return;

    // If there is data, there should be a device associated with it.
    ASSERT(device != VK_NULL_HANDLE && memory.has_value());
    vkUnmapMemory(device, *memory);
  }

  DELETE_COPY_AND_ASSIGN(MemoryBacked);

  MemoryBacked(MemoryBacked&& rhs) { Move(&rhs); }
  MemoryBacked& operator=(MemoryBacked&& rhs) {
    if (this != &rhs) {
      Move(&rhs);
    }
    return *this;
  }

  bool has_value() const { return handle.has_value() && memory.has_value(); }
  bool host_visible() const { return data != nullptr; }

  // Layout
  Handle<HandleType> handle;
  Handle<VkDeviceMemory> memory;

  // Not null if it's host visible.
  VkDeviceSize size;
  void* data = nullptr;

  VkDevice device = VK_NULL_HANDLE;

  private:
   void Move(MemoryBacked* rhs) {
     handle = std::move(rhs->handle);
     memory = std::move(rhs->memory);
     size = rhs->size;
     data = rhs->data;
     device = rhs->device;

     rhs->Reset();
   }

   void Reset() {
     data = nullptr;
     device = VK_NULL_HANDLE;
   }
};

}  // namespace vulkan
}  // namespace warhol
