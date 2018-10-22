// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <stdlib.h>

#include <utility>

#include <GL/gl3w.h>

namespace warhol {

const char* GLEnumToString(GLenum);
size_t GLEnumToSize(GLenum);

// Used to translate the unit (GL_TEXTURE0) to the name and index to be provided
// to the uniform.
std::pair<int, const char*> TextureUnitToUniform(GLenum);

}  // namespace warhol
