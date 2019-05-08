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

enum class SubShaderType {
  kVertex,
  kFragment,
};
const char* SubShaderTypeToString(SubShaderType type) {
  switch (type) {
    case SubShaderType::kVertex: return "Vertex";
    case SubShaderType::kFragment: return "Fragment";
  }

  NOT_REACHED() << "Invalid subshader type.";
  return nullptr;
}

struct ShaderParseResult {
  std::vector<uint8_t> source;
  std::vector<Uniform> uniforms;
};

std::vector<uint8_t> StringToSource(const std::string& str) {
  std::vector<uint8_t> src;
  src.reserve(str.size());
  src.insert(src.end(), str.begin(), str.end());
  return src;
}

bool ParseShader(const std::string_view& base_path, SubShaderType shader_type,
                 ShaderParseResult* out) {
  std::string shader_path;
  if (shader_type == SubShaderType::kVertex) {
    shader_path = StringPrintf("%s.vert", base_path.data());
  } else if (shader_type == SubShaderType::kFragment) {
    shader_path = StringPrintf("%s.frag", base_path.data());
  } else {
    LOG(ERROR) << "Invalid sub shader type.";
    return false;
  }

  std::string source;
  if (!ReadWholeFile(shader_path, &source))
    return false;

  *out = {};
  out->source = StringToSource(std::move(source));

  return true;
};

uint32_t GetUniformsSize(const std::vector<Uniform>& uniforms) {
  uint32_t result = 0;
  for (auto& uniform : uniforms) {
    result += uniform.size;
  }

  return result;
}

}  // namespace

uint64_t GetNextShaderUUID() { return kNextShaderUUID++; }

// Load Shader -----------------------------------------------------------------

namespace {

bool DoShaderParsing(BasePaths* paths, Renderer* renderer,
                     const std::string_view& shader_name,
                     const std::string_view& vert_name,
                     const std::string_view& frag_name,
                     Shader* shader) {
  if (renderer->type == RendererType::kVulkan) {
    LOG(ERROR) << "Vulkan shader type not supported yet.";
    return false;
  }

  ShaderParseResult vert_parse;
  if (!ParseShader(GetShaderPath(paths, vert_name), SubShaderType::kVertex,
                   &vert_parse)) {
    return false;
  }

  ShaderParseResult frag_parse;
  if (!ParseShader(GetShaderPath(paths, frag_name), SubShaderType::kFragment,
                   &frag_parse)) {
    return false;
  }

  shader->uuid = GetNextShaderUUID();
  shader->name = shader_name;

  shader->vert_source = std::move(vert_parse.source);
  shader->vert_ubo_size = GetUniformsSize(vert_parse.uniforms);
  shader->vert_uniforms = std::move(vert_parse.uniforms);

  shader->frag_source = std::move(frag_parse.source);
  shader->frag_ubo_size = GetUniformsSize(frag_parse.uniforms);
  shader->frag_uniforms = std::move(frag_parse.uniforms);

  // TODO(Cristian): Detect texture count.
  shader->texture_count = 0;

  return true;
}

}  // namespace

bool LoadShader(BasePaths* paths, Renderer* renderer,
                const std::string_view& shader_name,
                const std::string_view& vert_name,
                const std::string_view& frag_name,
                Shader* shader) {
  if (!DoShaderParsing(paths, renderer, shader_name, vert_name, frag_name,
                       shader)) {
    return false;
  }

  if (!RendererStageShader(renderer, shader)) {
    LOG(ERROR) << "Could not load shader " << shader_name;
    return false;
  }

  return true;
}

bool LoadShader(BasePaths* paths, Renderer* renderer,
                const std::string_view& shader_name,
                const std::string_view& filename,
                Shader* shader) {
  if (!LoadShader(paths, renderer, shader_name, filename, filename, shader)) {
    LOG(ERROR) << "Could not load shader " << shader_name;
    return false;
  }

  return true;
}


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
