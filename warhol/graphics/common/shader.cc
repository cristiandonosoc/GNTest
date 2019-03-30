// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/common/shader.h"

#include <atomic>

#include "warhol/utils/assert.h"
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

  NOT_REACHED("Invalid subshader type.");
  return nullptr;
}

struct ShaderParseResult {
  std::string source;
  std::vector<Uniform> uniforms;
};

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

  auto lines = SplitToLines(source);
  if (lines.empty()) {
    LOG(ERROR) << "Empty shader.";
    return false;
  }



  // Look for key parts of the shader.
  struct IncludeFile {
    size_t line_index;   // The line to be appended at.
    std::string source;
  };
  std::vector<IncludeFile> include_files;
  bool uniforms_found = false;
  std::vector<Uniform> uniforms;
  for (size_t i; i < lines.size(); i++) {
    auto& line = lines[i];
    if (line.find("//#INCLUDE") == 0) {
      std::string include_source;
      if (!ParseInclude(line, base_path, &include_source))
        return false;
      include_files.push_back({i, std::move(include_source)});
    } else if (line.find("UNIFORM_BLOCK") == 0) {
      if (uniforms_found) {
        LOG(ERROR) << "Uniform block already found.";
        return false;
      }

      uniforms_found = true;
      if (ParseUniforms(lines, i, &uniforms))
        return false;
    }
  }

  // Join the lines together.
  std::vector<std::string_view> new_lines;
  for (size_t i = 0; i < lines.size(); i++) {
    // We see if we need to append a include file.
    for (auto& include_file : include_files) {
      if (include_file.line_index == i) {
        new_lines.push_back(include_file.source);
        continue;
      }
    }

    new_lines.push_back(lines[i]);
  }


  out->source = Join(new_lines, "\n");
  out->uniforms = std::move(uniforms);
  return true;
};

uint32_t GetUniformsSize(const std::vector<Uniform>& uniforms) {
  uint32_t result;
  for (auto& uniform : uniforms) {
    result += uniform.size;
  }

  return result;
}

uint64_t GetNextShaderUUID() { return kNextShaderUUID++; }

bool LoadShader(const std::string_view& name,
                const std::string_view& base_path,
                Shader* shader) {
  ShaderParseResult vert_parse;
  if (!ParseShader(base_path, SubShaderType::kVertex, &vert_parse))
    return false;

  ShaderParseResult frag_parse;
  if (!ParseShader(base_path, SubShaderType::kFragment, &frag_parse))
    return false;

  shader->uuid = GetNextShaderUUID();
  shader->name = name;
  shader->path = base_path;

  shader->vert_source = std::move(vert_parse.source);
  shader->vert_ubo_size = GetUniformsSize(vert_parse.uniforms);
  shader->vert_uniforms = std::move(vert_parse.uniforms);

  shader->frag_source = std::move(frag_parse.source);
  shader->frag_ubo_size = GetUniformsSize(frag_parse.uniforms);
  shader->frag_uniforms = std::move(frag_parse.uniforms);

  return true;
}

void UnloadShader(Shader* shader) {
  shader->vert_source.clear();
  shader->frag_source.clear();
  shader->vert_ubo_size = -1;
  shader->frag_ubo_size = -1;
}

}  // namespace warhol
