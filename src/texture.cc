// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/texture.h"

#include <assert.h>

#include <third_party/stb/stb_image.h>

#include "src/graphics/GL/utils.h"
#include "src/shader.h"
#include "src/utils/log.h"

namespace warhol {

Texture::Texture(std::string path) {
  data_.path = std::move(path);

  Init();
}

Texture::Texture(Texture&& other) {
  *this = std::move(other);
}

Texture::~Texture() {
  if (handle_)
    glDeleteTextures(1, &handle_);
}

Texture& Texture::operator=(Texture&& other) {
  if (this != &other) {
    data_ = std::move(other.data_);
    handle_ = other.handle_;
    other.handle_ = 0;
  }
  return *this;
}

void Texture::Set(Shader* shader, GLenum tex_unit) const {
	assert(valid());
  GL_CALL(glActiveTexture, tex_unit);
	GL_CALL(glBindTexture, GL_TEXTURE_2D, handle_);

  auto [unit_index, unit_name] = TextureUnitToUniform(tex_unit);
  shader->SetInt(unit_name, unit_index);
}

void Texture::Disable(GLenum tex_unit) {
  GL_CALL(glActiveTexture, tex_unit);
  GL_CALL(glBindTexture, tex_unit, NULL);
  GL_CALL(glDisable, GL_TEXTURE_2D);
  GL_CALL(glActiveTexture, NULL);
}

void
Texture::Init() {
  // So that this matches what OpenGL expects.
  stbi_set_flip_vertically_on_load(true);

  uint8_t* data = stbi_load(path().data(),
                            &data_.x, &data_.y, &data_.channels,
                            0);
  if (!data)
    LOG(ERROR) << "Could not load texture " << path();

  GL_CALL(glGenTextures, 1, &handle_);
  GL_CALL(glBindTexture, GL_TEXTURE_2D, handle_);

  // Setup wrapping/filtering options.
  GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Format
  GLenum format = channels() == 3 ? GL_RGB : GL_RGBA;

  // Send the bits over.
  // TODO(Cristian): Check for errors.
  // TODO(Cristian): When multi-threading is done, use this async.
  //                 Think what to do about in memory buffer.
  GL_CALL(glTexImage2D, GL_TEXTURE_2D,     // target
                        0,                 // level
                        format,            // internalformat
                        data_.x,           // width,
                        data_.y,           // height
                        0,                 // border
                        format,            // format
                        GL_UNSIGNED_BYTE,  // type,
                        data);
  GL_CALL(glGenerateMipmap, GL_TEXTURE_2D);

  // Now we free the texture.
  stbi_image_free(data);
}

}  // namespace warhol
