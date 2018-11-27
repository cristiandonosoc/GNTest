// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/graphics/GL/debug.h"

#include <stdio.h>

#include "src/graphics/GL/def.h"
#include "src/graphics/GL/utils.h"

namespace warhol {

bool LogGLError(const char* file, int line, const char* context) {
  GLenum err = glGetError();
  bool error_found = false;
  while (err != GL_NO_ERROR) {
    fprintf(stderr, "[ERROR][%s:%d] [%s] OpenGL Error %s\n",
            file, line, context, GLEnumToString(err));
    error_found = true;
    err = glGetError();
  }
  return error_found;
}

uint32_t GetGLError(const char** error_name) {
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
    *error_name = GLEnumToString(error);
  return (uint32_t) error;
}

}  // namespace warhol


