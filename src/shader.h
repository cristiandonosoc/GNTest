// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>

#include "utils/status.h"

namespace warhol {


class Shader {
 public:
  Shader();
  Shader(std::string vert_src, std::string frag_src);
  ~Shader();

  Status Init();
  bool valid() const { return handle_ != 0; }

 private:
  // Clears all the handles that the shader owns.
  void Clear();
  // So that Init can clear resources on failure.
  Status InternalInit();

  int handle_ = 0;
  int vert_handle_ = 0;
  int frag_handle_ = 0;

  std::string vert_src_;
  std::string frag_src_;
};

}  // namespace warhol
