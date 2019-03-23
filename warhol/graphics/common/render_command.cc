// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/common/render_command.h"

#include "warhol/utils/assert.h"

namespace warhol {

RenderCommandsManager::~RenderCommandsManager() {
  if (Valid(this))
    ShutdownRenderCommandsManager(this);
}

bool InitRenderCommandsManager(RenderCommandsManager* manager) {
  manager->pool = std::make_unique<uint8_t[]>(1024 * 1024);
  return true;
}

void SHutdownRenderCommandsManager(RenderCommandsManager* manager) {
  ASSERT(Valid(manager));
  manager->pool.reset();
}

}  // namespace warhol
