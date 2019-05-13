// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/common/shader.h"

#include <atomic>
#include <map>

#include "warhol/assets/asset_paths.h"
#include "warhol/graphics/common/renderer.h"
#include "warhol/utils/file.h"
#include "warhol/utils/log.h"
#include "warhol/utils/string.h"

namespace warhol {

namespace {

std::atomic<uint64_t> kNextShaderUUID = 1;

}  // namespace

uint64_t GetNextShaderUUID() { return kNextShaderUUID++; }

void RemoveSources(Shader* shader) {
  shader->vert_source.clear();
  shader->frag_source.clear();
}

std::string ShaderSourceAsString(const std::vector<uint8_t>& src) {
  std::string str;
  str.reserve(src.size());
  str.insert(str.end(), src.begin(), src.end());
  return str;
}

// CalculateUniformLayout ------------------------------------------------------

namespace {

struct UniformLayout {
  uint32_t size = 0;
  uint32_t alignment = 0;
};

const std::map<UniformType, UniformLayout>& GetLayout() {
  static std::map<UniformType, UniformLayout> layout = {
    { UniformType::kBool, {4, 4}},
    { UniformType::kInt, {4, 4}},
    { UniformType::kFloat, {4, 4}},

    { UniformType::kInt2, {8, 8}},
    { UniformType::kInt3, {12, 16}},
    { UniformType::kInt4, {16, 16}},

    { UniformType::kUint2, {8, 8}},
    { UniformType::kUint3, {12, 16}},
    { UniformType::kUint4, {16, 16}},

    { UniformType::kVec2, {8, 8}},
    { UniformType::kVec3, {12, 16}},
    { UniformType::kVec4, {16, 16}},

    { UniformType::kMat4, {64, 16}},
  };

  return layout;
}

uint32_t NextMultiple(uint32_t val, uint32_t multiple) {
  if (multiple == 0)
    return val;

  uint32_t remainder = val % multiple;
  if (remainder == 0)
    return val;
  return val + multiple - remainder;
}

}  // namespace

bool CalculateUniformLayout(std::vector<Uniform>* uniforms) {


  uint32_t current_offset = 0;
  for (Uniform& uniform : *uniforms) {
    if (!Valid(&uniform)) {
      LOG(ERROR) << "Uniform " << uniform.name << " is not valid.";
      return false;
    }

    const auto& uniforms_layout = GetLayout();
    auto it = uniforms_layout.find(uniform.type);
    if (it == uniforms_layout.end())
      return false;

    auto& layout = it->second;

    uniform.size = layout.size;
    uniform.alignment = layout.alignment;

    uint32_t next_start = NextMultiple(current_offset, uniform.alignment);
    uniform.offset = next_start;
    current_offset = uniform.offset + uniform.size;
  }

  return true;
}

}  // namespace warhol
