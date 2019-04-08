// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/opengl/texture.h"

#include "warhol/graphics/common/texture.h"
#include "warhol/graphics/opengl/renderer_backend.h"

namespace warhol {
namespace opengl {

bool OpenGLStageTexture(OpenGLRendererBackend* opengl, Texture* texture) {
  uint64_t uuid = texture->uuid.value;
  auto it = opengl->loaded_textures.find(uuid);
  if (it != opengl->loaded_textures.end()) {
    LOG(ERROR) << "Shader " << texture->name << " is already loaded.";
    return false;
  }

  uint32_t handle;
  GL_CHECK(glGenTextures(1, &handle));
  GL_CHECK(glBindTexture(GL_TEXTURE_2D, handle));

  // Setup wrapping/filtering options.
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

  // Send the bits over.
  // TODO(Cristian): Check for errors.
  // TODO(Cristian): When multi-threading is done, use this async.
  //                 Think what to do about in memory buffer.
  GL_CHECK(glTexImage2D(GL_TEXTURE_2D,          // target
                        0,                      // level
                        GL_RGBA,                // internalformat
                        texture->x,             // width,
                        texture->y,             // height
                        0,                      // border
                        GL_RGBA,                // format
                        GL_UNSIGNED_BYTE,       // type,
                        texture->data.value));
  GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D));

  TextureHandles handles;
  handles.tex_handle = handle;
  opengl->loaded_textures[uuid] = std::move(handles);
  return true;
}

void OpenGLUnstageTexture(OpenGLRendererBackend* opengl, Texture* texture) {
  auto it = opengl->loaded_textures.find(texture->uuid.value);
  ASSERT(it != opengl->loaded_textures.end());

  GL_CHECK(glDeleteTextures(1, &it->second.tex_handle));
  opengl->loaded_textures.erase(it);
}

}  // namespace opengl
}  // namespace warhol
