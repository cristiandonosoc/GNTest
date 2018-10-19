// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/texture.h"

#include <assert.h>

#include <third_party/stb/stb_image.h>

#include "src/shader.h"
#include "src/utils/gl.h"
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
  glEnable(tex_unit);
  glActiveTexture(tex_unit);
	glBindTexture(GL_TEXTURE_2D, handle_);

  // TODO(Cristian): enable C++17 so I can use tuple expansion.
  auto index = TextureUnitToUniform(tex_unit);
  shader->SetInt(index.second, index.first);
  glActiveTexture(NULL);
}

void Texture::Disable(GLenum tex_unit) {
  glActiveTexture(tex_unit);
  glBindTexture(tex_unit, NULL);
  glDisable(GL_TEXTURE_2D);
  glActiveTexture(NULL);
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

  glGenTextures(1, &handle_);
  glBindTexture(GL_TEXTURE_2D, handle_);

  // Setup wrapping/filtering options.
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Format
  GLenum format = channels() == 3 ? GL_RGB : GL_RGBA;


  // Send the bits over.
  // TODO(Cristian): Check for errors.
  // TODO(Cristian): When multi-threading is done, use this async.
  //                 Think what to do about in memory buffer.
  glTexImage2D(GL_TEXTURE_2D,     // target
               0,                 // level
               GL_RGB,            // internalformat
               data_.x,           // width,
               data_.y,           // height
               0,                 // border
               format,            // format
               GL_UNSIGNED_BYTE,  // type,
               data);
  glGenerateMipmap(GL_TEXTURE_2D);

  // Now we free the texture.
  stbi_image_free(data);
}

}  // namespace warhol
