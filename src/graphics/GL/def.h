// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

// For getting GL symbols, now we import this header instead on relying that we
// are going to always use gl3w (which we probably always will, but still...
// it's good to be future proof).

#include <stdio.h>
#include <stdlib.h>

#include <utility>

#include <GL/gl3w.h>

#include "src/graphics/GL/debug.h"

namespace warhol {

#define GL_CALL(func, ...) \
  ::warhol::GL::Call(func, #func, __FILE__, __LINE__, __VA_ARGS__)

struct GL {
  static void Init();

  template <typename FunctionType, typename... Args>
  static inline void Call(FunctionType func, const char* func_str,
                          const char* file, int line,
                          Args&&... args) {
    func(std::forward<Args>(args)...);
    const char* error_name;
    GLenum error = (GLenum)GetGLError(&error_name);
    if (error != GL_NO_ERROR) {
      fprintf(stderr, "[ERROR][%s:%d] When calling %s: %s\n",
                      file, line, func_str, error_name);
      exit(1);
    }
  }
};

}  // namespace warhol
