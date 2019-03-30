// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdlib.h>

#include <GL/gl3w.h>

namespace warhol {
namespace opengl {

// Used to define a new variable name for each GL_CHECK call.
#define GL_VAR COMBINE(gl_err, __LINE__)

// Wraps an opengl call with an error checking query.
// Will assert on error.
#define GL_CHECK(opengl_call)                            \
  {                                                      \
    opengl_call;                                         \
    GLenum GL_VAR = glGetError();                        \
    if (GL_VAR != GL_NO_ERROR) {                         \
      auto location = FROM_HERE;                         \
      fprintf(stderr,                                    \
              "[ERROR][%s:%d] When calling %s: %s\n",    \
              location.file,                             \
              location.line,                             \
              #opengl_call,                              \
              ::warhol::opengl::GLEnumToString(GL_VAR)); \
      NOT_REACHED("Invalid OpenGL call. See logs.");     \
    }                                                    \
  }

const char* GLEnumToString(GLenum);
size_t GLEnumToSize(GLenum);

}  // namespace opengl
}  // namespace warhol
