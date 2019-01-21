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

enum class MemoryUsage {
  kNone,
  kGPUOnly,
  kCPUOnly,
  kCPUToGPU,
  kGPUToCPU,
};
const char* MemoryUsageToString(MemoryUsage);

// |memory_type_bits| determines what kind of memory we can use.
// This is normally given by |vkGetBufferMemoryRequirements| or its image
// equivalent.
// Returns UINT32_MAX on error.
uint32_t FindMemoryTypeIndex(Context*, MemoryUsage, uint32_t memory_type_bits);

}  // namespace vulkan
}  // namespace warhol
