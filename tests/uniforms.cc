// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <warhol/graphics/common/shader.h>

#include <third_party/catch2/catch.hpp>

namespace warhol {
namespace test {

namespace {

Uniform CreateUniform(UniformType type) {
  Uniform uniform;
  uniform.type = type;
  return uniform;
}

}  // namespace

TEST_CASE("CalculateUniformLayout") {
  std::vector<Uniform> uniforms = {
    CreateUniform(UniformType::kFloat),
    CreateUniform(UniformType::kVec3),
    CreateUniform(UniformType::kMat4),
    CreateUniform(UniformType::kVec2),
    CreateUniform(UniformType::kVec3),
    CreateUniform(UniformType::kVec4),
    CreateUniform(UniformType::kBool),
    CreateUniform(UniformType::kBool),
    CreateUniform(UniformType::kMat4),
    CreateUniform(UniformType::kInt),
  };

  REQUIRE(CalculateUniformLayout(&uniforms));

  // Float.
  REQUIRE(uniforms[0].alignment == 4);
  REQUIRE(uniforms[0].offset == 0);
  REQUIRE(uniforms[0].size == 4);

  // Vec3.
  REQUIRE(uniforms[1].alignment == 16);
  REQUIRE(uniforms[1].offset == 16);
  REQUIRE(uniforms[1].size == 12);

  // Mat4.
  REQUIRE(uniforms[2].alignment == 16);
  REQUIRE(uniforms[2].offset == 32);
  REQUIRE(uniforms[2].size == 64);

  // Vec2.
  REQUIRE(uniforms[3].alignment == 8);
  REQUIRE(uniforms[3].offset == 96);
  REQUIRE(uniforms[3].size == 8);

  // Vec3.
  REQUIRE(uniforms[4].alignment == 16);
  REQUIRE(uniforms[4].offset == 112);
  REQUIRE(uniforms[4].size == 12);

  // Vec4.
  REQUIRE(uniforms[5].alignment == 16);
  REQUIRE(uniforms[5].offset == 128);
  REQUIRE(uniforms[5].size == 16);

  // Bool.
  REQUIRE(uniforms[6].alignment == 4);
  REQUIRE(uniforms[6].offset == 144);
  REQUIRE(uniforms[6].size == 4);

  // Bool.
  REQUIRE(uniforms[7].alignment == 4);
  REQUIRE(uniforms[7].offset == 148);
  REQUIRE(uniforms[7].size == 4);

  // Mat4.
  REQUIRE(uniforms[8].alignment == 16);
  REQUIRE(uniforms[8].offset == 160);
  REQUIRE(uniforms[8].size == 64);

  // Int.
  REQUIRE(uniforms[9].alignment == 4);
  REQUIRE(uniforms[9].offset == 224);
  REQUIRE(uniforms[9].size == 4);
}

}  // namespace test
}  // namespace warhol
