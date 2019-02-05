// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

namespace warhol {
namespace vulkan {

struct RendererBackend;

// Collection of functions that implement the low-level functionality exposed
// by the vulkan renderer backend.
// This is mainly an ordering separation, as this all could be implemented in
// renderer_backend.cc as non-exported functions.


void StartFrame(RendererBackend*);
void EndFrame(RendererBackend*);

}  // namespace vulkan
}  // namespace warhol
