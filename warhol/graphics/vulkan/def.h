// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

// This def header file should be the only way to get definitions for vulkan
// in warhol. This is so that if we need some platform specific voodoo magic
// to find the headers, we can case it here and leave the other code alone.

#include <vulkan/vulkan.h>

namespace warhol {
namespace vulkan {

constexpr int kMaxFramesInFlight = 2;

// TODO(Cristian): Actually use this when creating framebuffers.
constexpr int kNumFrames = 2;

}  // namespace vulkan
}  // namespace warhol
