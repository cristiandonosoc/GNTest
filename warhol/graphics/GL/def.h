// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

// For getting GL symbols, now we import this header instead on relying that we
// are going to always use gl3w (which we probably always will, but still...
// it's good to be future proof).

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <utility>

#include <GL/gl3w.h>

#include "warhol/graphics/GL/debug.h"
#include "warhol/utils/location.h"

namespace warhol {

#define GL_CALL(func, ...) \
  ::warhol::GL::Call(func, #func, FROM_HERE, __VA_ARGS__)

// Will query the set contextual thread-local location.
#define CTX_GL_CALL(func, ...)                                                 \
  ::warhol::GL::Call(func,                                                     \
                     #func,                                                    \
                     Location::GetThreadCurrentLocation({__FILE__, __LINE__}), \
                     __VA_ARGS__)

struct GL {
  static bool Init();

  template <typename FunctionType, typename... Args>
  static inline void Call(FunctionType func,
                          const char* func_str,
                          const Location& location,
                          Args&&... args) {
    func(std::forward<Args>(args)...);
    const char* error_name;
    GLenum error = (GLenum)GetGLError(&error_name);
    if (error != GL_NO_ERROR) {
      fprintf(stderr,
              "[ERROR][%s:%d] When calling %s: %s\n",
              location.file,
              location.line,
              func_str,
              error_name);
      exit(1);
    }
  }
};

}  // namespace warhol
