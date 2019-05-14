// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/common/shader.h"

#include <stdint.h>
#include <third_party/cpptoml/cpptoml.h>

#include "warhol/assets/asset_paths.h"
#include "warhol/graphics/opengl/renderer_backend.h"
#include "warhol/graphics/opengl/utils.h"
#include "warhol/platform/path.h"
#include "warhol/utils/file.h"
#include "warhol/utils/log.h"

namespace warhol {
namespace opengl {


// Parse Shader ----------------------------------------------------------------

namespace {

enum class SubShaderType {
  kConfig,
  kFragment,
  kVertex,
};

std::vector<uint8_t> StringToSource(const std::string& str) {
  std::vector<uint8_t> src;
  src.reserve(str.size());
  src.insert(src.end(), str.begin(), str.end());
  return src;
}

uint32_t GetUniformsSize(const std::vector<Uniform>& uniforms) {
  if (uniforms.empty())
    return 0;
  auto& back = uniforms.back();
  return back.offset + back.size;
}

std::optional<std::string> GetSubShaderPath(const std::string& basename,
                             SubShaderType type) {
  std::string shader_path;
  if (type == SubShaderType::kVertex) {
    return StringPrintf("%s.vert", basename.c_str());
  } else if (type== SubShaderType::kFragment) {
    return StringPrintf("%s.frag", basename.c_str());
  } else if (type == SubShaderType::kConfig) {
    return StringPrintf("%s.toml", basename.c_str());
  }
  LOG(ERROR) << "Invalid sub shader type.";

  return std::nullopt;
}

struct ShaderLayout {
  std::vector<Uniform> vert_uniforms;
  std::vector<Uniform> frag_uniforms;
};

std::optional<Uniform> ParseUniform(const std::string& name,
                                    const std::string& type_str) {
  UniformType type = FromString(type_str);
  if (type == UniformType::kLast)
    return std::nullopt;

  Uniform uniform;
  uniform.name = name;
  uniform.type = type;
  uniform.size = GetSize(type);
  uniform.alignment = GetAlignment(type);

  return uniform;
}

std::optional<std::vector<Uniform>>
ParseUniforms(std::shared_ptr<cpptoml::table_array> toml_table) {
  std::vector<Uniform> uniforms;
  for (const auto& entry : *toml_table) {
    auto name = entry->get_as<std::string>("name");
    if (!name) {
      LOG(ERROR) << "Could not find name in toml table";
      return std::nullopt;
    }

    auto type = entry->get_as<std::string>("type");
    if (!type) {
      LOG(ERROR) << "Could not find type in toml table";
      return std::nullopt;
    }

    auto uniform = ParseUniform(*name, *type);
    if (!uniform) {
      LOG(ERROR) << "Could not parse uniform " << *name;
      return std::nullopt;
    }
  }

  return uniforms;
}

std::map<std::string, ShaderLayout>& GetShaderLayouts() {
  static std::map<std::string, ShaderLayout> layouts;
  return layouts;
}

bool LoadShaderLayout(const std::string& layout_filepath) {
  static std::set<std::string> loaded_files;
  auto file_it = loaded_files.find(layout_filepath);
  if (file_it != loaded_files.end()) {
    LOG(DEBUG) << "Layout file " << layout_filepath << " already loaded!";
    return true;
  }
  loaded_files.insert(layout_filepath);

  auto& layouts = GetShaderLayouts();

  try {
    auto layout_file = cpptoml::parse_file(layout_filepath.c_str());
    if (!layout_file) {
      LOG(ERROR) << "Could not parse file " << layout_filepath;
      return false;
    }

    for (const auto& [key, toml_entry] : *layout_file) {
      auto entry_it = layouts.find(key);
      if (entry_it != layouts.end()) {
        LOG(ERROR) << "Shader layout for " << key << " already found!";
        return false;
      }

      LOG(DEBUG) << "Loading layout for shader " << key;
      auto& shader_layout = layouts[key];

      // Parse vertex.
      auto vert_entries = toml_entry->as_table()->get_table_array("vert");
      if (vert_entries) {
        if (auto uniforms = ParseUniforms(vert_entries); uniforms) {
          shader_layout.vert_uniforms = std::move(*uniforms);
        } else {
          LOG(ERROR) << "Could not load vert entries for " << key;
          return false;
        }
      }

      // Parse fragment.
      auto frag_entries = toml_entry->as_table()->get_table_array("frag");
      if (frag_entries) {
        if (auto uniforms = ParseUniforms(frag_entries); uniforms) {
          shader_layout.vert_uniforms = std::move(*uniforms);
        } else {
          LOG(ERROR) << "Could not load vert entries for " << key;
          return false;
        }
      }
    }
  } catch (cpptoml::parse_exception& e) {
    LOG(ERROR) << "Error parsing toml file " << layout_filepath << ": "
               << e.what();
    return false;
  }

  return true;
}

struct ParseOut {
  std::string source;
  std::vector<Uniform> uniforms;
};
std::optional<ParseOut>
ParseSubShader(BasePaths* paths, const std::string& name, SubShaderType type) {
  const auto& layouts = GetShaderLayouts();
  auto layout_it = layouts.find(name);
  if (layout_it == layouts.end()) {
    NOT_REACHED() << "Could not find layout for " << name;
    return std::nullopt;
  }

  LOG(DEBUG) << "Found layout for " << name;

  const char* ext = type == SubShaderType::kVertex ? "vert" : "frag";
  std::string path = PathJoin({paths->shader,
                               StringPrintf("%s.%s", name.c_str(), ext)});
  std::string source;
  if (!ReadWholeFile(path, &source))
    return std::nullopt;

  ParseOut out;
  out.source = std::move(source);
  auto& layout = layout_it->second;
  out.uniforms = type == SubShaderType::kVertex ? layout.vert_uniforms
                                                : layout.frag_uniforms;

  if (!CalculateUniformLayout(&out.uniforms))
    return std::nullopt;
  return out;
}

}  // namespace

bool OpenGLParseShader(BasePaths* paths,
                       const std::string& vert_name,
                       const std::string& frag_name,
                       Shader* shader) {
  if (!LoadShaderLayout(PathJoin({paths->shader, "layouts.toml"}))) {
    NOT_REACHED() << "Parsing toml file.";
    return false;
  }
  auto vert_out = ParseSubShader(paths, vert_name, SubShaderType::kVertex);
  if (!vert_out) {
    LOG(ERROR) << "Could not parse vertex shader: " << vert_name;
    return false;
  }

  auto frag_out = ParseSubShader(paths, frag_name, SubShaderType::kFragment);
  if (!frag_out) {
    LOG(ERROR) << "Could not parse fragment shader: " << frag_name;
    return false;
  }

  shader->uuid = GetNextShaderUUID();

  shader->vert_source = std::move(vert_out->source);
  shader->vert_ubo_size = GetUniformsSize(vert_out->uniforms);
  shader->vert_uniforms = std::move(vert_out->uniforms);

  shader->frag_source = std::move(frag_out->source);
  shader->frag_ubo_size = GetUniformsSize(frag_out->uniforms);
  shader->frag_uniforms = std::move(frag_out->uniforms);

  // TODO(Cristian): Detect texture count.
  shader->texture_count = 0;

  return true;
}

// Stage Shader ----------------------------------------------------------------

namespace {

bool CompileShader(Shader* shader, const char* source, GLenum shader_kind,
                   uint32_t* out_handle) {
  uint32_t handle = glCreateShader(shader_kind);
  if (!handle) {
    LOG(ERROR) << "Shader " << shader->name << ": Could not allocate shader.";
    return false;
  }

  // Compile the shader source.
  const GLchar* gl_src = source;
  glShaderSource(handle, 1, &gl_src, 0);
  glCompileShader(handle);

  GLint success = 0;
  glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
  if (success == GL_FALSE) {
    GLchar log[2048];
    glGetShaderInfoLog(handle, sizeof(log), 0, log);
    glDeleteShader(handle);
    LOG(ERROR) << "Shader " << shader->name << ": Error compiling "
               << GLEnumToString(shader_kind) << " shader: " << log;
    return false;
  }

  *out_handle = handle;
  return true;
}

bool CompileVertShader(Shader* shader, uint32_t* out_handle) {
  return CompileShader(shader, (char*)shader->vert_source.data(),
                       GL_VERTEX_SHADER, out_handle);
}

bool CompileFragShader(Shader* shader, uint32_t* out_handle) {
  return CompileShader(shader,
                       (char*)shader->frag_source.data(),
                       GL_FRAGMENT_SHADER,
                       out_handle);
}

// Returns the program handle or 0.
uint32_t LinkShader(uint32_t vert_handle, uint32_t frag_handle) {
  uint32_t prog_handle = glCreateProgram();
  if (prog_handle == 0) {
    LOG(ERROR) << "glCreateProgram: could not allocate a program";
    return 0;
  }

  // Link 'em.
  glAttachShader(prog_handle, vert_handle);
  glAttachShader(prog_handle, frag_handle);
  glLinkProgram(prog_handle);
  glDeleteShader(vert_handle);
  glDeleteShader(frag_handle);

  GLint success = 0;
  glGetProgramiv(prog_handle, GL_LINK_STATUS, &success);
  if (success == GL_FALSE) {
    GLchar log[2048];
    glGetProgramInfoLog(prog_handle, sizeof(log), 0, log);
    LOG(ERROR) << "Could not link shader: " << log;
    return 0;
  }

  return prog_handle;
}

// Warhol's shaders are structured into known uniform blocks, which need to be
// bound to pre-known binding indices.
bool LinkUniformBinding(const char* block_name, uint32_t prog_handle,
                        uint32_t binding) {
  uint32_t block_index = glGetUniformBlockIndex(prog_handle, block_name);
  if (block_index == GL_INVALID_INDEX) {
    return false;
  }

  glUniformBlockBinding(prog_handle, block_index, binding);
  return true;
}

// Create a uniform buffer and bind it.
uint32_t BindBufferBase(uint32_t binding) {
  uint32_t ubo_handle = 0;
  glGenBuffers(1, &ubo_handle);
  glBindBufferBase(GL_UNIFORM_BUFFER, binding, ubo_handle);
  return ubo_handle;
}

bool InternalUploadShader(Shader* shader, ShaderHandles* handles) {
  *handles = {};  // Clear out.

  // Compile the shaders and and program.
  uint32_t vert_handle = 0;
  uint32_t frag_handle = 0;
  if (!CompileVertShader(shader, &vert_handle) ||
      !CompileFragShader(shader, &frag_handle)) {
    return false;
  }

  uint32_t prog_handle = LinkShader(vert_handle, frag_handle);
  if (prog_handle == 0)
    return false;

  handles->program_handle = prog_handle;

  // Get the uniform buffer blocks.
  if (!LinkUniformBinding("Camera", prog_handle, 0)) {
    LOG(ERROR) << "Uniform block binding is not optional.";
    return false;
  }
  handles->camera_binding = 0;

  if (LinkUniformBinding("VertUniforms", prog_handle, 1)) {
    handles->vert_ubo_binding = 1;
    handles->vert_ubo_handle = BindBufferBase(1);
  }

  if (LinkUniformBinding("FragUniforms", prog_handle, 2)) {
    handles->frag_ubo_binding = 2;
    handles->frag_ubo_handle = BindBufferBase(2);
  }

  return true;
}

bool UploadShader(Shader* shader, ShaderHandles* shader_desc) {
  bool result = InternalUploadShader(shader, shader_desc);
  if (!result) {
    DeleteShaderHandles(shader_desc);
    return false;
  }

  LOG(DEBUG) << "Uploaded shader. " << std::endl
             << "[VERT] Handle: " << shader_desc->vert_ubo_handle
             << ", Binding: " << shader_desc->vert_ubo_binding << std::endl
             << "[FRAG] Handle: " << shader_desc->frag_ubo_handle
             << ", Binding: " << shader_desc->frag_ubo_binding << std::endl;

  return true;
}

}  // namespace

// Shader Handling -------------------------------------------------------------

bool OpenGLStageShader(OpenGLRendererBackend* opengl, Shader* shader) {
  uint64_t uuid = shader->uuid.value;
  auto it = opengl->loaded_shaders.find(uuid);
  if (it != opengl->loaded_shaders.end()) {
    LOG(ERROR) << "Shader " << shader->name << " is already loaded.";
    return false;
  }

  ShaderHandles handles;
  if (!UploadShader(shader, &handles))
    return false;

  opengl->loaded_shaders[uuid] = std::move(handles);
  return true;
}

void OpenGLUnstageShader(OpenGLRendererBackend* opengl, Shader* shader) {
  auto it = opengl->loaded_shaders.find(shader->uuid.value);
  ASSERT(it != opengl->loaded_shaders.end());

  DeleteShaderHandles(&it->second);
  opengl->loaded_shaders.erase(it);
}

void DeleteShaderHandles(ShaderHandles* handles) {
  if (handles->program_handle > 0)
    glDeleteProgram(handles->program_handle);

  if (handles->vert_ubo_handle > 0)
    glDeleteBuffers(1, &handles->vert_ubo_handle);

  if (handles->frag_ubo_handle > 0)
    glDeleteBuffers(1, &handles->frag_ubo_handle);

  *handles = {};    // Clear.
}

}  // namespace opengl
}  // namespace warhol
