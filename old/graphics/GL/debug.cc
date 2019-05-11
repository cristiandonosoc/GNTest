// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/GL/debug.h"

#include <stdio.h>

#include "warhol/graphics/GL/def.h"
#include "warhol/graphics/GL/utils.h"

namespace warhol {

uint32_t GetGLError(const char** error_name) {
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
    *error_name = GLEnumToString(error);
  return (uint32_t) error;
}

}  // namespace warhol


