// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/common/render_command.h"

#include "warhol/utils/string.h"

namespace warhol {

std::string ToString(IndexRange range) {
  return StringPrintf("Size: %u, Offset: %u", GetSize(range), GetOffset(range));
}

// For all constructors, we move the first linked list, as they're all the same
// in the fact that they contain no side-effects POD.

RenderCommand::~RenderCommand() = default;

}  // namespace warhol
