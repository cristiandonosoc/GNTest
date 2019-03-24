// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdlib.h>

#include <GL/gl3w.h>

namespace warhol {
namespace opengl {

const char* GLEnumToString(GLenum);
size_t GLEnumToSize(GLenum);

}  // namespace opengl
}  // namespace warhol
