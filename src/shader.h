// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <map>
#include <string>

#include "src/graphics/GL/utils.h"
#include "src/utils/macros.h"
#include "src/utils/glm.h"

namespace warhol {

struct ShaderString {
  const char* value;
};

// Used to translate the unit (GL_TEXTURE0) to the name and index to be provided
// to the uniform.
std::pair<int, ShaderString> TextureUnitToUniform(GLenum);

class Shader {
 public:
  struct Attribute {
    std::string name;
    int location;
    GLenum type;
    size_t count;  // Whether it's an array.
    size_t size;   // The size of the type in bytes.

    static ShaderString kPos;
    static ShaderString kColor;
    static ShaderString kTexCoord0;
    static ShaderString kTexCoord1;
  };

  struct Uniform {
    std::string name;
    int location;
    GLenum type;
    size_t count;  // Whether it's an array.
    size_t size;   // The size of the type in bytes.

    static ShaderString kModel;
    static ShaderString kView;
    static ShaderString kProjection;
    static ShaderString kTexSampler0;
    static ShaderString kTexSampler1;
    static ShaderString kTexSampler2;
    static ShaderString kTexSampler3;
    static ShaderString kTexSampler4;
    static ShaderString kTexSampler5;
    static ShaderString kTexSampler6;
    static ShaderString kTexSampler7;
    static ShaderString kTexSampler8;
    static ShaderString kTexSampler9;
    static ShaderString kTexSampler10;
    static ShaderString kTexSampler11;
    static ShaderString kTexSampler12;
    static ShaderString kTexSampler13;
    static ShaderString kTexSampler14;
    static ShaderString kTexSampler15;
  };

  // Passes in the asset paths to Assets::ShaderPath. Then it attemps to read
  // the files and create the shader.
  //
  // If any errors ocurred, they will be logged and the returned shader valid()
  // will return false.
  static Shader FromAssetPath(std::string name,
                              std::string vert_asset_path,
                              std::string frag_asset_path);

  Shader();
  Shader(std::string name, std::string vert_src, std::string frag_src);
  ~Shader();

  DELETE_COPY_AND_ASSIGN(Shader);
  DEFAULT_MOVE_AND_ASSIGN(Shader);

  bool Init();
  bool valid() const { return program_handle_.handle != 0; }

  void Use();

  const Attribute* GetAttribute(ShaderString name) const;
  const std::map<std::string, Attribute>& attributes() const {
    return attributes_;
  }

  const Uniform* GetUniform(ShaderString name) const;
  const std::map<std::string, Uniform>& uniforms() const { return uniforms_; }

  bool SetInt(ShaderString name, int);
  bool SetFloat(ShaderString name, float);
  bool SetMat4(ShaderString, const glm::mat4&);

  const std::string& name() const { return name_; }

  uint32_t program_handle() const { return program_handle_.handle; }


 private:
  // Clears all the handles that the shader owns.
  void Clear();
  // So that Init can clear resources on failure.
  bool InternalInit();

  // Internal function to set up the matrices.
  bool SetMatrix(ShaderString name, size_t mat_length, const float* data);

  void ObtainAttributes();
  void ObtainUniforms();

  std::string name_;
  std::string vert_src_;
  std::string frag_src_;

  // The handles free the resource on destruction.
  GLHandle<GL_VERTEX_SHADER> vert_handle_;
  GLHandle<GL_FRAGMENT_SHADER> frag_handle_;
  GLHandle<GL_PROGRAM> program_handle_;

  std::map<std::string, Uniform> uniforms_;
  std::map<std::string, Attribute> attributes_;
};

}  // namespace warhol
