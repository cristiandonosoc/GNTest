// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/common/render_command.h"

#include "warhol/utils/assert.h"

namespace warhol {

const char* RenderCommand::TypeToString(RenderCommand::Type type) {
  switch (type) {
    case RenderCommand::Type::kMesh: return "Mesh";
    case RenderCommand::Type::kLast: return "Last";
  }

  NOT_REACHED("Unknown RenderCommand::Type");
  return nullptr;
}

}  // namespace warhol
