// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/common/shader.h"

#include <atomic>

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

}  // namespace warhol
