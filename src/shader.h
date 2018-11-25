// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <map>
#include <string>

#include "src/graphics/GL/utils.h"
#include "src/utils/macros.h"
#include "src/utils/glm.h"

namespace warhol {

struct Uniform {
  std::string name;
  int location;
  GLenum type;
  size_t count;   // Whether it's an array.
  size_t size;    // The size of the type in bytes.
};

struct Attribute {
  std::string name;
  int location;
  GLenum type;
  size_t count;   // Whether it's an array.
  size_t size;    // The size of the type in bytes.
};

class Shader {
 public:
  struct Attributes {
    static const char* kModel;
  };

  Shader();
  Shader(std::string vert_src, std::string frag_src);
  ~Shader();

  DELETE_COPY_AND_ASSIGN(Shader);
  DEFAULT_MOVE_AND_ASSIGN(Shader);

  bool Init();
  bool valid() const { return program_handle_.handle != 0; }

  void Use();

  const Attribute* GetAttribute(const std::string& name) const;
  const std::map<std::string, Attribute>& attributes() const {
    return attributes_;
  }

  const Uniform* GetUniform(const std::string& name) const;
  const std::map<std::string, Uniform>& uniforms() const { return uniforms_; }

  bool SetInt(const std::string& name, int);
  bool SetFloat(const std::string& name, float);

  bool SetMat4(const std::string&, const glm::mat4&);

  // NOTE: This is a square matrix.

 private:
  // Clears all the handles that the shader owns.
  void Clear();
  // So that Init can clear resources on failure.
  bool InternalInit();

  // Internal function to set up the matrices.
  bool SetMatrix(const std::string&, size_t mat_length, const float* data);

  void ObtainAttributes();
  void ObtainUniforms();

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
