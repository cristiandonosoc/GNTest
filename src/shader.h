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

 private:
  int handle_ = 0;
  int vert_handle_ = 0;
  int frag_handle_ = 0;

  std::string vert_src_;
  std::string frag_src_;
};

}  // namespace warhol
