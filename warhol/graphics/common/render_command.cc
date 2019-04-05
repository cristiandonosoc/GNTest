// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/common/render_command.h"

namespace warhol {

// For all constructors, we move the first linked list, as they're all the same
// in the fact that they contain no side-effects POD.

RenderCommand::Actions::Actions() {
  mesh_actions = {};
}

RenderCommand::Actions::~Actions() {
  Reset(&mesh_actions);
}

RenderCommand::Actions::Actions(const Actions& other) {
  mesh_actions = other.mesh_actions;
}

RenderCommand::Actions&
RenderCommand::Actions::operator=(const Actions& other) {
  if (this == &other)
    return *this;
  mesh_actions = other.mesh_actions;
  return *this;
}

RenderCommand::Actions::Actions(Actions&& other) {
  mesh_actions = std::move(other.mesh_actions);
}

RenderCommand::Actions&
RenderCommand::Actions::operator=(Actions&& other) {
  if (this == &other)
    return *this;
  mesh_actions = std::move(other.mesh_actions);
  return *this;
}

}  // namespace warhol
