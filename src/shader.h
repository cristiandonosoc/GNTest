// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <map>
#include <string>

#include <GL/gl3w.h>

#include "utils/macros.h"
#include "utils/status.h"

BEGIN_IGNORE_WARNINGS()
#include <third_party/include/glm/glm.hpp>
END_IGNORE_WARNINGS()

namespace warhol {

struct Uniform {
  std::string name;

  int location;
  GLenum type;
  size_t count;   // Whether it's an array.
  size_t size;    // The size of the type in bytes.
};

class Shader {
 public:
  Shader();
  Shader(std::string vert_src, std::string frag_src);
  ~Shader();

  Status Init();
  bool valid() const { return handle_ != 0; }

  void Use();

  const Uniform* GetUniform(const std::string& uniform_name) const;
  const std::map<std::string, Uniform> uniforms() const { return uniforms_; }

  bool SetInt(const std::string& name, int) const;
  bool SetFloat(const std::string& name, float) const;

  bool SetMat4(const std::string&, const glm::mat4&);

  // NOTE: This is a square matrix.

 private:
  // Clears all the handles that the shader owns.
  void Clear();
  // So that Init can clear resources on failure.
  Status InternalInit();

  // Internal function to set up the matrices.
  bool SetMatrix(const std::string&, size_t mat_length, const float* data);

  void ObtainUniforms();

  int handle_ = 0;
  int vert_handle_ = 0;
  int frag_handle_ = 0;

  std::string vert_src_;
  std::string frag_src_;

  std::map<std::string, Uniform> uniforms_;

  DELETE_COPY_AND_ASSIGN(Shader);
};

}  // namespace warhol
