// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <utility>

#include "warhol/graphics/vulkan/allocator.h"
#include "warhol/graphics/vulkan/handle.h"
#include "warhol/graphics/vulkan/def.h"

namespace warhol {

struct Image;

namespace vulkan {

template <typename HandleType>
struct MemoryBacked;

// Memory ----------------------------------------------------------------------

Handle<VkDeviceMemory>
AllocMemory(Context* context, const VkMemoryRequirements& memory_reqs,
            const VkMemoryPropertyFlags& desired_properties);

Handle<VkDeviceMemory>
AllocMemory(Context*, VkDeviceSize size, uint32_t memory_type_index);

// VkBuffer --------------------------------------------------------------------

// Creates a VkBuffer and allocates it.
struct AllocBufferConfig {
  VkDeviceSize size;
  VkBufferUsageFlags usage;
  MemoryUsage memory_usage = MemoryUsage::kNone;
};
MemoryBacked<VkBuffer> AllocBuffer(Context*, AllocBufferConfig*);

// Copies one buffer and blocks waiting for the transfer.
bool CopyBuffer(Context* context, VkBuffer src_buffer, VkBuffer dst_buffer,
                VkDeviceSize size);

// MemoryBacked Handles --------------------------------------------------------
//
// Represents a handle (VkBuffer, VkImage, etc.) that is backed by some memory
// memory in the GPU (VkDeviceMemory). This struct holds the memory as a
// resource and will destroy it on destruction.

template <typename HandleType>
struct MemoryBacked {
  // Constructors
  MemoryBacked() = default;

  DELETE_COPY_AND_ASSIGN(MemoryBacked);

  MemoryBacked(MemoryBacked&& rhs) { Move(&rhs); }
  MemoryBacked& operator=(MemoryBacked&& rhs) {
    if (this != &rhs) {
      Move(&rhs);
    }
    return *this;
  }

  bool has_value() const {
    return handle.has_value() && allocation.has_value();
  }
  bool host_visible() const { return allocation.host_visible(); }
  void* data() { return allocation.data; }

  // Layout
  Handle<HandleType> handle;
  Allocation allocation = {};

  // Not null if it's host visible.
  VkDeviceSize size;

  VkDevice device = VK_NULL_HANDLE;

  private:
   void Move(MemoryBacked* rhs) {
     handle = std::move(rhs->handle);
     allocation = std::move(rhs->allocation);
     size = rhs->size;
     device = rhs->device;

     rhs->Reset();
   }

   void Reset() {
     device = VK_NULL_HANDLE;
     allocation = {};
   }
};

}  // namespace vulkan
}  // namespace warhol
