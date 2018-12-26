// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/GL/def.h"

#include <stdio.h>

#include "warhol/utils/log.h"

namespace warhol {

namespace {

const char*
Gl3wInitResultToString(int res) {
  switch (res) {
    case GL3W_OK: return "GL3W_OK";
    case GL3W_ERROR_INIT: return "GL3W_ERROR_INIT";
    case GL3W_ERROR_LIBRARY_OPEN: return "GL3W_ERROR_LIBRARY_OPEN";
    case GL3W_ERROR_OPENGL_VERSION: return "GL3W_ERROR_OPENGL_VERSION";
    default:
      break;
  }
  LOG(WARNING) << "Got unknown GL3W init result: " << res;
  return "";
}

}  // namespace

bool GL::Init() {
  int res = gl3wInit();
  if (res != GL3W_OK)
    LOG(WARNING) << "Got non-OK GL3W result: " << Gl3wInitResultToString(res);
  return true;
}

}  // namespace warhol
