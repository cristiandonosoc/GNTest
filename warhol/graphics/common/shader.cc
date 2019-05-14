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

uint32_t GetSize(UniformType type) {
  switch (type) {
    case UniformType::kBool: return 4;
    case UniformType::kInt: return 4;
    case UniformType::kFloat: return 4;
    case UniformType::kInt2: return 8;
    case UniformType::kInt3: return 12;
    case UniformType::kInt4: return 16;
    case UniformType::kUint2: return 8;
    case UniformType::kUint3: return 12;
    case UniformType::kUint4: return 16;
    case UniformType::kVec2: return 8;
    case UniformType::kVec3: return 12;
    case UniformType::kVec4: return 16;
    case UniformType::kIvec2: return 8;
    case UniformType::kIvec3: return 12;
    case UniformType::kIvec4: return 16;
    case UniformType::kUvec2: return 8;
    case UniformType::kUvec3: return 12;
    case UniformType::kUvec4: return 16;
    case UniformType::kMat4: return 64;
    case UniformType::kLast: break;
  }

  NOT_REACHED() << "Should not ask for invalid type";
  return 0;
}

uint32_t GetAlignment(UniformType type) {
  switch (type) {
    case UniformType::kBool: return 4;
    case UniformType::kInt: return 4;
    case UniformType::kFloat: return 4;
    case UniformType::kInt2: return 8;
    case UniformType::kInt3: return 16;
    case UniformType::kInt4: return 16;
    case UniformType::kUint2: return 8;
    case UniformType::kUint3: return 16;
    case UniformType::kUint4: return 16;
    case UniformType::kVec2: return 8;
    case UniformType::kVec3: return 16;
    case UniformType::kVec4: return 16;
    case UniformType::kIvec2: return 8;
    case UniformType::kIvec3: return 16;
    case UniformType::kIvec4: return 16;
    case UniformType::kUvec2: return 8;
    case UniformType::kUvec3: return 16;
    case UniformType::kUvec4: return 16;
    case UniformType::kMat4: return 16;
    case UniformType::kLast: break;
  }

  NOT_REACHED() << "Should not ask for invalid type";
  return 0;
}

UniformType FromString(const std::string& type_str) {
  UniformType type = UniformType::kLast;
  if (type_str == "bool") { type = UniformType::kBool; }
  else if (type_str == "int") { type = UniformType::kInt; }
  else if (type_str == "float") { type = UniformType::kFloat; }
  else if (type_str == "int2") { type = UniformType::kInt2; }
  else if (type_str == "int3") { type = UniformType::kInt3; }
  else if (type_str == "int4") { type = UniformType::kInt4; }
  else if (type_str == "uint2") { type = UniformType::kUint2; }
  else if (type_str == "uint3") { type = UniformType::kUint3; }
  else if (type_str == "uint4") { type = UniformType::kUint4; }
  else if (type_str == "vec2") { type = UniformType::kVec2; }
  else if (type_str == "vec3") { type = UniformType::kVec3; }
  else if (type_str == "vec4") { type = UniformType::kVec4; }
  else if (type_str == "ivec2") { type = UniformType::kIvec2; }
  else if (type_str == "ivec3") { type = UniformType::kIvec3; }
  else if (type_str == "ivec4") { type = UniformType::kIvec4; }
  else if (type_str == "uvec2") { type = UniformType::kUvec2; }
  else if (type_str == "uvec3") { type = UniformType::kUvec3; }
  else if (type_str == "uvec4") { type = UniformType::kUvec4; }
  else if (type_str == "mat4") { type = UniformType::kMat4; }

  return type;
}

// CalculateUniformLayout ------------------------------------------------------

namespace {

struct UniformLayout {
  uint32_t size = 0;
  uint32_t alignment = 0;
};

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

    uniform.size = GetSize(uniform.type);
    uniform.alignment = GetAlignment(uniform.type);

    uint32_t next_start = NextMultiple(current_offset, uniform.alignment);
    uniform.offset = next_start;
    current_offset = uniform.offset + uniform.size;
  }

  return true;
}

}  // namespace warhol
