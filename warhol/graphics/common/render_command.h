// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

namespace warhol {

struct Camera;
struct Mesh;

struct RenderCommand {
  enum class Type {
    kMesh,
    kLast,
  };
  static const char* TypeToString(Type);

  Type type = Type::kLast;
  Mesh* mesh = nullptr;
  Camera* camera = nullptr;
};

}  // namespace warhol
