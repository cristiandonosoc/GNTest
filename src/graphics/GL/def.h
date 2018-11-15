// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

// For getting GL symbols, now we import this header instead on relying that we
// are going to always use gl3w (which we probably always will, but still...
// it's good to be future proof).

/* #ifdef __APPLE__ */
/* #include <OpenGL/gl.h> */
/* #else */
/* #include <GL/gl3w.h> */
/* #endif */
#include <GL/gl3w.h>

namespace warhol {

struct GL {
  static void Init();
};

}  // namespace warhol
