// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <memory>

#include "warhol/containers/linked_list.h"
#include "warhol/graphics/common/texture.h"

namespace warhol {

struct Camera;
struct Mesh;
struct Shader;

enum class ShaderID {
  kCommon,
  kLast,
};
const char* ToString(ShaderID);

// A Shader can have any combination of this uniforms.
//
// The set is determined by the shader uniform layout.
enum class Uniform : uint32_t {
  kModelMatrixX,
  kModelMatrixY,
  kModelMatrixZ,
  kModelMatrixW,

  kViewMatrixX,
  kViewMatrixY,
  kViewMatrixZ,
  kViewMatrixW,

  kProjectionMatrixX,
  kProjectionMatrixY,
  kProjectionMatrixZ,
  kProjectionMatrixW,

  kUser0,
  kUser1,
  kUser2,
  kUser3,
  kUser4,
  kUser5,
  kUser6,
  kUser7,

  kLast,
};
const char* ToString(Uniform);

// Where a particular texture is bound.
enum class TextureBind {
  kTex0,
  kTex1,
  kTex2,
  kTex3,
  kLast,
};
const char* ToString(TextureBind);

struct UniformValue {
  Uniform uniform;
  union {
    struct { float r, g, b, a; };
    struct { float x, y, z, w; };
    float values[4];
  };
};

struct MeshRenderAction {
  Mesh* mesh = nullptr;

  UniformValue* values = nullptr;
  uint32_t value_count = 0;


};

enum class RenderCommandType {
  kMesh,
  kLast,
};

struct RenderCommand {
  RenderCommandType type = RenderCommandType::kLast;

  Camera* camera;
  Shader* shader;
  union {
    LinkedList<MeshRenderAction>* mesh_actions;
  };
};

}  // namespace warhol
