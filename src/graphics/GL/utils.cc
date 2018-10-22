// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/graphics/GL/utils.h"

#include <assert.h>
#include <stdio.h>

#include "src/utils/log.h"

namespace warhol {

const char*
GLEnumToString(GLenum type) {
  switch (type) {
    case GL_FLOAT: return "GL_FLOAT";
    case GL_FLOAT_VEC2: return "GL_FLOAT_VEC2";
    case GL_FLOAT_VEC3: return "GL_FLOAT_VEC3";
    case GL_FLOAT_VEC4: return "GL_FLOAT_VEC4";
    case GL_DOUBLE: return "GL_DOUBLE";
    case GL_DOUBLE_VEC2: return "GL_DOUBLE_VEC2";
    case GL_DOUBLE_VEC3: return "GL_DOUBLE_VEC3";
    case GL_DOUBLE_VEC4: return "GL_DOUBLE_VEC4";
    case GL_INT: return "GL_INT";
    case GL_INT_VEC2: return "GL_INT_VEC2";
    case GL_INT_VEC3: return "GL_INT_VEC3";
    case GL_INT_VEC4: return "GL_INT_VEC4";
    case GL_UNSIGNED_INT: return "GL_UNSIGNED_INT";
    case GL_UNSIGNED_INT_VEC2: return "GL_UNSIGNED_INT_VEC2";
    case GL_UNSIGNED_INT_VEC3: return "GL_UNSIGNED_INT_VEC3";
    case GL_UNSIGNED_INT_VEC4: return "GL_UNSIGNED_INT_VEC4";
    case GL_BOOL: return "GL_BOOL";
    case GL_BOOL_VEC2: return "GL_BOOL_VEC2";
    case GL_BOOL_VEC3: return "GL_BOOL_VEC3";
    case GL_BOOL_VEC4: return "GL_BOOL_VEC4";
    case GL_FLOAT_MAT2: return "GL_FLOAT_MAT2";
    case GL_FLOAT_MAT3: return "GL_FLOAT_MAT3";
    case GL_FLOAT_MAT4: return "GL_FLOAT_MAT4";
    case GL_FLOAT_MAT2x3: return "GL_FLOAT_MAT2x3";
    case GL_FLOAT_MAT2x4: return "GL_FLOAT_MAT2x4";
    case GL_FLOAT_MAT3x2: return "GL_FLOAT_MAT3x2";
    case GL_FLOAT_MAT3x4: return "GL_FLOAT_MAT3x4";
    case GL_FLOAT_MAT4x2: return "GL_FLOAT_MAT4x2";
    case GL_FLOAT_MAT4x3: return "GL_FLOAT_MAT4x3";
    case GL_DOUBLE_MAT2: return "GL_DOUBLE_MAT2";
    case GL_DOUBLE_MAT3: return "GL_DOUBLE_MAT3";
    case GL_DOUBLE_MAT4: return "GL_DOUBLE_MAT4";
    case GL_DOUBLE_MAT2x3: return "GL_DOUBLE_MAT2x3";
    case GL_DOUBLE_MAT2x4: return "GL_DOUBLE_MAT2x4";
    case GL_DOUBLE_MAT3x2: return "GL_DOUBLE_MAT3x2";
    case GL_DOUBLE_MAT3x4: return "GL_DOUBLE_MAT3x4";
    case GL_DOUBLE_MAT4x2: return "GL_DOUBLE_MAT4x2";
    case GL_DOUBLE_MAT4x3: return "GL_DOUBLE_MAT4x3";
    case GL_SAMPLER_1D: return "GL_SAMPLER_1D";
    case GL_SAMPLER_2D: return "GL_SAMPLER_2D";
    case GL_SAMPLER_3D: return "GL_SAMPLER_3D";
    case GL_SAMPLER_CUBE: return "GL_SAMPLER_CUBE";
    case GL_SAMPLER_1D_SHADOW: return "GL_SAMPLER_1D_SHADOW";
    case GL_SAMPLER_2D_SHADOW: return "GL_SAMPLER_2D_SHADOW";
    case GL_SAMPLER_1D_ARRAY: return "GL_SAMPLER_1D_ARRAY";
    case GL_SAMPLER_2D_ARRAY: return "GL_SAMPLER_2D_ARRAY";
    case GL_SAMPLER_1D_ARRAY_SHADOW: return "GL_SAMPLER_1D_ARRAY_SHADOW";
    case GL_SAMPLER_2D_ARRAY_SHADOW: return "GL_SAMPLER_2D_ARRAY_SHADOW";
    case GL_SAMPLER_2D_MULTISAMPLE: return "GL_SAMPLER_2D_MULTISAMPLE";
    case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
      return "GL_SAMPLER_2D_MULTISAMPLE_ARRAY";
    case GL_SAMPLER_CUBE_SHADOW: return "GL_SAMPLER_CUBE_SHADOW";
    case GL_SAMPLER_BUFFER: return "GL_SAMPLER_BUFFER";
    case GL_SAMPLER_2D_RECT: return "GL_SAMPLER_2D_RECT";
    case GL_SAMPLER_2D_RECT_SHADOW: return "GL_SAMPLER_2D_RECT_SHADOW";
    case GL_INT_SAMPLER_1D: return "GL_INT_SAMPLER_1D";
    case GL_INT_SAMPLER_2D: return "GL_INT_SAMPLER_2D";
    case GL_INT_SAMPLER_3D: return "GL_INT_SAMPLER_3D";
    case GL_INT_SAMPLER_CUBE: return "GL_INT_SAMPLER_CUBE";
    case GL_INT_SAMPLER_1D_ARRAY: return "GL_INT_SAMPLER_1D_ARRAY";
    case GL_INT_SAMPLER_2D_ARRAY: return "GL_INT_SAMPLER_2D_ARRAY";
    case GL_INT_SAMPLER_2D_MULTISAMPLE: return "GL_INT_SAMPLER_2D_MULTISAMPLE";
    case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
      return "GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY";
    case GL_INT_SAMPLER_BUFFER: return "GL_INT_SAMPLER_BUFFER";
    case GL_INT_SAMPLER_2D_RECT: return "GL_INT_SAMPLER_2D_RECT";
    case GL_UNSIGNED_INT_SAMPLER_1D: return "GL_UNSIGNED_INT_SAMPLER_1D";
    case GL_UNSIGNED_INT_SAMPLER_2D: return "GL_UNSIGNED_INT_SAMPLER_2D";
    case GL_UNSIGNED_INT_SAMPLER_3D: return "GL_UNSIGNED_INT_SAMPLER_3D";
    case GL_UNSIGNED_INT_SAMPLER_CUBE: return "GL_UNSIGNED_INT_SAMPLER_CUBE";
    case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
      return "GL_UNSIGNED_INT_SAMPLER_1D_ARRAY";
    case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
      return "GL_UNSIGNED_INT_SAMPLER_2D_ARRAY";
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
      return "GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE";
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
      return "GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY";
    case GL_UNSIGNED_INT_SAMPLER_BUFFER:
      return "GL_UNSIGNED_INT_SAMPLER_BUFFER";
    case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
      return "GL_UNSIGNED_INT_SAMPLER_2D_RECT";
    case GL_IMAGE_1D: return "GL_IMAGE_1D";
    case GL_IMAGE_2D: return "GL_IMAGE_2D";
    case GL_IMAGE_3D: return "GL_IMAGE_3D";
    case GL_IMAGE_2D_RECT: return "GL_IMAGE_2D_RECT";
    case GL_IMAGE_CUBE: return "GL_IMAGE_CUBE";
    case GL_IMAGE_BUFFER: return "GL_IMAGE_BUFFER";
    case GL_IMAGE_1D_ARRAY: return "GL_IMAGE_1D_ARRAY";
    case GL_IMAGE_2D_ARRAY: return "GL_IMAGE_2D_ARRAY";
    case GL_IMAGE_2D_MULTISAMPLE: return "GL_IMAGE_2D_MULTISAMPLE";
    case GL_IMAGE_2D_MULTISAMPLE_ARRAY: return "GL_IMAGE_2D_MULTISAMPLE_ARRAY";
    case GL_INT_IMAGE_1D: return "GL_INT_IMAGE_1D";
    case GL_INT_IMAGE_2D: return "GL_INT_IMAGE_2D";
    case GL_INT_IMAGE_3D: return "GL_INT_IMAGE_3D";
    case GL_INT_IMAGE_2D_RECT: return "GL_INT_IMAGE_2D_RECT";
    case GL_INT_IMAGE_CUBE: return "GL_INT_IMAGE_CUBE";
    case GL_INT_IMAGE_BUFFER: return "GL_INT_IMAGE_BUFFER";
    case GL_INT_IMAGE_1D_ARRAY: return "GL_INT_IMAGE_1D_ARRAY";
    case GL_INT_IMAGE_2D_ARRAY: return "GL_INT_IMAGE_2D_ARRAY";
    case GL_INT_IMAGE_2D_MULTISAMPLE: return "GL_INT_IMAGE_2D_MULTISAMPLE";
    case GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
      return "GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY";
    case GL_UNSIGNED_INT_IMAGE_1D: return "GL_UNSIGNED_INT_IMAGE_1D";
    case GL_UNSIGNED_INT_IMAGE_2D: return "GL_UNSIGNED_INT_IMAGE_2D";
    case GL_UNSIGNED_INT_IMAGE_3D: return "GL_UNSIGNED_INT_IMAGE_3D";
    case GL_UNSIGNED_INT_IMAGE_2D_RECT: return "GL_UNSIGNED_INT_IMAGE_2D_RECT";
    case GL_UNSIGNED_INT_IMAGE_CUBE: return "GL_UNSIGNED_INT_IMAGE_CUBE";
    case GL_UNSIGNED_INT_IMAGE_BUFFER: return "GL_UNSIGNED_INT_IMAGE_BUFFER";
    case GL_UNSIGNED_INT_IMAGE_1D_ARRAY:
      return "GL_UNSIGNED_INT_IMAGE_1D_ARRAY";
    case GL_UNSIGNED_INT_IMAGE_2D_ARRAY:
      return "GL_UNSIGNED_INT_IMAGE_2D_ARRAY";
    case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE:
      return "GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE";
    case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
      return "GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY";
    case GL_UNSIGNED_INT_ATOMIC_COUNTER:
      return "GL_UNSIGNED_INT_ATOMIC_COUNTER";
    case GL_VERTEX_SHADER: return "GL_VERTEX_SHADER";
    case GL_FRAGMENT_SHADER: return "GL_FRAGMENT_SHADER";
    case GL_LINK_STATUS: return "GL_LINK_STATUS";

    case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
    case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
    case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      return "GL_INVALID_FRAMEBUFFER_OPERATION";

    default:
      LOG(ERROR) << "Uncovered GLenum type: " << (uint32_t)type;
      assert(false);
      return nullptr;
  }
}

size_t
GLEnumToSize(GLenum type) {
  switch (type) {
    case GL_FLOAT: return 1 * sizeof(GLfloat);
    case GL_FLOAT_VEC2: return 2 * sizeof(GLfloat);
    case GL_FLOAT_VEC3: return 3 * sizeof(GLfloat);
    case GL_FLOAT_VEC4: return 4 * sizeof(GLfloat);
    case GL_DOUBLE: return 1 * sizeof(GLdouble);
    case GL_DOUBLE_VEC2: return 2 * sizeof(GLdouble);
    case GL_DOUBLE_VEC3: return 3 * sizeof(GLdouble);
    case GL_DOUBLE_VEC4: return 4 * sizeof(GLdouble);
    case GL_INT: return 1 * sizeof(GLint);
    case GL_INT_VEC2: return 2 * sizeof(GLint);
    case GL_INT_VEC3: return 3 * sizeof(GLint);
    case GL_INT_VEC4: return 4 * sizeof(GLint);
    case GL_UNSIGNED_INT: return 1 * sizeof(GLuint);
    case GL_UNSIGNED_INT_VEC2: return 2 * sizeof(GLuint);
    case GL_UNSIGNED_INT_VEC3: return 3 * sizeof(GLuint);
    case GL_UNSIGNED_INT_VEC4: return 4 * sizeof(GLuint);
    case GL_BOOL: return 1 * sizeof(GLboolean);
    case GL_BOOL_VEC2: return 2 * sizeof(GLboolean);
    case GL_BOOL_VEC3: return 3 * sizeof(GLboolean);
    case GL_BOOL_VEC4: return 4 * sizeof(GLboolean);
    case GL_FLOAT_MAT2: return 2 * 2 * sizeof(GLfloat);
    case GL_FLOAT_MAT3: return 3 * 3 * sizeof(GLfloat);
    case GL_FLOAT_MAT4: return 4 * 4 * sizeof(GLfloat);
    case GL_FLOAT_MAT2x3: return 2 * 3 * sizeof(GLfloat);
    case GL_FLOAT_MAT2x4: return 2 * 4 * sizeof(GLfloat);
    case GL_FLOAT_MAT3x2: return 3 * 2 * sizeof(GLfloat);
    case GL_FLOAT_MAT3x4: return 3 * 4 * sizeof(GLfloat);
    case GL_FLOAT_MAT4x2: return 4 * 2 * sizeof(GLfloat);
    case GL_FLOAT_MAT4x3: return 4 * 3 * sizeof(GLfloat);
    case GL_DOUBLE_MAT2: return 2 * 2 * sizeof(GLdouble);
    case GL_DOUBLE_MAT3: return 3 * 3 * sizeof(GLdouble);
    case GL_DOUBLE_MAT4: return 4 * 4 * sizeof(GLdouble);
    case GL_DOUBLE_MAT2x3: return 2 * 3 * sizeof(GLdouble);
    case GL_DOUBLE_MAT2x4: return 2 * 4 * sizeof(GLdouble);
    case GL_DOUBLE_MAT3x2: return 3 * 2 * sizeof(GLdouble);
    case GL_DOUBLE_MAT3x4: return 3 * 4 * sizeof(GLdouble);
    case GL_DOUBLE_MAT4x2: return 4 * 2 * sizeof(GLdouble);
    case GL_DOUBLE_MAT4x3: return 4 * 3 * sizeof(GLdouble);
    case GL_SAMPLER_1D: return 1 * sizeof(uint32_t);
    case GL_SAMPLER_2D: return 1 * sizeof(uint32_t);
    case GL_SAMPLER_3D: return 1 * sizeof(uint32_t);

#ifdef GLENUM_TO_SIZE_COMPLETE_TYPES_AS_NEEDED
    case GL_SAMPLER_CUBE: return;
    case GL_SAMPLER_1D_SHADOW: return;
    case GL_SAMPLER_2D_SHADOW: return;
    case GL_SAMPLER_1D_ARRAY: return;
    case GL_SAMPLER_2D_ARRAY: return;
    case GL_SAMPLER_1D_ARRAY_SHADOW: return;
    case GL_SAMPLER_2D_ARRAY_SHADOW: return;
    case GL_SAMPLER_2D_MULTISAMPLE: return;
    case GL_SAMPLER_2D_MULTISAMPLE_ARRAY: return;
    case GL_SAMPLER_CUBE_SHADOW: return;
    case GL_SAMPLER_BUFFER: return;
    case GL_SAMPLER_2D_RECT: return;
    case GL_SAMPLER_2D_RECT_SHADOW: return;
    case GL_INT_SAMPLER_1D: return;
    case GL_INT_SAMPLER_2D: return;
    case GL_INT_SAMPLER_3D: return;
    case GL_INT_SAMPLER_CUBE: return;
    case GL_INT_SAMPLER_1D_ARRAY: return;
    case GL_INT_SAMPLER_2D_ARRAY: return;
    case GL_INT_SAMPLER_2D_MULTISAMPLE: return;
    case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY: return;
    case GL_INT_SAMPLER_BUFFER: return;
    case GL_INT_SAMPLER_2D_RECT: return;
    case GL_UNSIGNED_INT_SAMPLER_1D: return;
    case GL_UNSIGNED_INT_SAMPLER_2D: return;
    case GL_UNSIGNED_INT_SAMPLER_3D: return;
    case GL_UNSIGNED_INT_SAMPLER_CUBE: return;
    case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY: return;
    case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY: return;
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE: return;
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY: return;
    case GL_UNSIGNED_INT_SAMPLER_BUFFER: return;
    case GL_UNSIGNED_INT_SAMPLER_2D_RECT: return;
    case GL_IMAGE_1D: return;
    case GL_IMAGE_2D: return;
    case GL_IMAGE_3D: return;
    case GL_IMAGE_2D_RECT: return;
    case GL_IMAGE_CUBE: return;
    case GL_IMAGE_BUFFER: return;
    case GL_IMAGE_1D_ARRAY: return;
    case GL_IMAGE_2D_ARRAY: return;
    case GL_IMAGE_2D_MULTISAMPLE: return;
    case GL_IMAGE_2D_MULTISAMPLE_ARRAY: return;
    case GL_INT_IMAGE_1D: return;
    case GL_INT_IMAGE_2D: return;
    case GL_INT_IMAGE_3D: return;
    case GL_INT_IMAGE_2D_RECT: return;
    case GL_INT_IMAGE_CUBE: return;
    case GL_INT_IMAGE_BUFFER: return;
    case GL_INT_IMAGE_1D_ARRAY: return;
    case GL_INT_IMAGE_2D_ARRAY: return;
    case GL_INT_IMAGE_2D_MULTISAMPLE: return;
    case GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY: return;
    case GL_UNSIGNED_INT_IMAGE_1D: return;
    case GL_UNSIGNED_INT_IMAGE_2D: return;
    case GL_UNSIGNED_INT_IMAGE_3D: return;
    case GL_UNSIGNED_INT_IMAGE_2D_RECT: return;
    case GL_UNSIGNED_INT_IMAGE_CUBE: return;
    case GL_UNSIGNED_INT_IMAGE_BUFFER: return;
    case GL_UNSIGNED_INT_IMAGE_1D_ARRAY: return;
    case GL_UNSIGNED_INT_IMAGE_2D_ARRAY: return;
    case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE: return;
    case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY: return;
    case GL_UNSIGNED_INT_ATOMIC_COUNTER: return;
#endif
    default:
      LOG(ERROR) << "Uncovered GLenum type: " << (uint32_t)type;
      assert(false);
      return 0;
  }
}

std::pair<int, const char*> TextureUnitToUniform(GLenum tex) {
  switch (tex) {
    case GL_TEXTURE0: return {0, "tex0"};
    case GL_TEXTURE1: return {1, "tex1"};
    case GL_TEXTURE2: return {2, "tex2"};
    case GL_TEXTURE3: return {3, "tex3"};
    case GL_TEXTURE4: return {4, "tex4"};
    case GL_TEXTURE5: return {5, "tex5"};
    case GL_TEXTURE6: return {6, "tex6"};
    case GL_TEXTURE7: return {7, "tex7"};
    case GL_TEXTURE8: return {8, "tex8"};
    case GL_TEXTURE9: return {9, "tex9"};
    case GL_TEXTURE10: return {10, "tex10"};
    case GL_TEXTURE11: return {11, "tex11"};
    case GL_TEXTURE12: return {12, "tex12"};
    case GL_TEXTURE13: return {13, "tex13"};
    case GL_TEXTURE14: return {14, "tex14"};
    case GL_TEXTURE15: return {15, "tex15"};
    default:
      LOG(ERROR) << "Invalid texture given: " << GLEnumToString(tex);
      assert(false);
      return {0, nullptr};
  }
}

bool
CheckGLErrors(const char* file, int line, const char* context) {
  GLenum err = glGetError();
  bool error_found = false;
  while (err != GL_NO_ERROR) {
    fprintf(stderr, "[ERROR][%s:%d] OpenGL Error %s: %s\n", file, line,
                    GLEnumToString(err), context);
    error_found = true;
    err = glGetError();
  }
  return error_found;
}

}  // namespace warhol
