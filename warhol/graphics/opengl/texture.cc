// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/opengl/texture.h"

#include "warhol/graphics/common/renderer.h"
#include "warhol/graphics/common/texture.h"
#include "warhol/graphics/opengl/renderer_backend.h"

namespace warhol {
namespace opengl {

namespace {

GLenum WrapToGL(StageTextureConfig::Wrap wrap) {
  switch (wrap) {
    case StageTextureConfig::Wrap::kClampToEdge: return GL_CLAMP_TO_EDGE;
    case StageTextureConfig::Wrap::kMirroredRepeat: return GL_MIRRORED_REPEAT;
    case StageTextureConfig::Wrap::kRepeat: return GL_REPEAT;
  }

  NOT_REACHED();
  return 0;
}

GLenum FilterToGL(StageTextureConfig::Filter filter) {
  switch (filter) {
    case StageTextureConfig::Filter::kNearest: return GL_NEAREST;
    case StageTextureConfig::Filter::kLinear: return GL_LINEAR;
    case StageTextureConfig::Filter::kNearestMipmapNearest:
      return GL_NEAREST_MIPMAP_NEAREST;
    case StageTextureConfig::Filter::kNearestMipmapLinear:
      return GL_NEAREST_MIPMAP_LINEAR;
    case StageTextureConfig::Filter::kLinearMipmapNearest:
      return GL_LINEAR_MIPMAP_NEAREST;
    case StageTextureConfig::Filter::kLinearMipampLinear:
      return GL_LINEAR_MIPMAP_LINEAR;
  }

  NOT_REACHED();
  return 0;
}

}  // namespace

bool OpenGLStageTexture(OpenGLRendererBackend* opengl, Texture* texture,
                        StageTextureConfig* config) {
  uint64_t uuid = texture->uuid.value;
  auto it = opengl->loaded_textures.find(uuid);
  if (it != opengl->loaded_textures.end()) {
    LOG(ERROR) << "Shader " << texture->name << " is already loaded.";
    return false;
  }

  uint32_t handle;
  glGenTextures(1, &handle);
  glBindTexture(GL_TEXTURE_2D, handle);

  // Setup wrapping/filtering options.
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                  WrapToGL(config->wrap_u));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                  WrapToGL(config->wrap_v));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  FilterToGL(config->min_filter));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                  FilterToGL(config->max_filter));

  // Send the bits over.
  glTexImage2D(GL_TEXTURE_2D,         // target
               0,                     // level
               GL_RGBA,               // internalformat
               texture->x,            // width,
               texture->y,            // height
               0,                     // border
               GL_RGBA,               // format
               GL_UNSIGNED_BYTE,      // type,
               texture->data.value);

  if (config->generate_mipmaps)
    glGenerateMipmap(GL_TEXTURE_2D);

  TextureHandles handles;
  handles.tex_handle = handle;
  opengl->loaded_textures[uuid] = std::move(handles);

  glBindTexture(GL_TEXTURE_2D, NULL);
  return true;
}

void OpenGLUnstageTexture(OpenGLRendererBackend* opengl, Texture* texture) {
  auto it = opengl->loaded_textures.find(texture->uuid.value);
  ASSERT(it != opengl->loaded_textures.end());

  glDeleteTextures(1, &it->second.tex_handle);
  opengl->loaded_textures.erase(it);
}

}  // namespace opengl
}  // namespace warhol
