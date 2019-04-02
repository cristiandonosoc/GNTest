// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/common/texture.h"

#include <atomic>

#include <third_party/stb/stb_image.h>

#include "warhol/utils/assert.h"
#include "warhol/utils/log.h"

namespace warhol {

namespace {

std::atomic<uint64_t> kNextTextureUUID = 1;

};

uint64_t GetNextTextureUUID() { return kNextTextureUUID++; }

Texture::~Texture() {
  if (Valid(this))
    UnloadTexture(this);
}

bool LoadTexture(const std::string& path, TextureType texture_type,
                 Texture* out_texture) {
  // OpenGL expects the Y axis to be inverted.
  if (texture_type == TextureType::kOpenGL) {
    stbi_set_flip_vertically_on_load(true);
  } else {
    stbi_set_flip_vertically_on_load(false);
  }

  Texture tmp;
  tmp.data = stbi_load(path.c_str(), &tmp.x, &tmp.y, &tmp.channels,
                       STBI_rgb_alpha);
  if (!tmp.data.value) {
    LOG(ERROR) << "Could not load texture in " << path << ": "
               << stbi_failure_reason();
    return false;
  }

  *out_texture = std::move(tmp);
  out_texture->free_function = stbi_image_free;
  out_texture->uuid = GetNextTextureUUID();
  return true;
}

void UnloadTexture(Texture* texture) {
  ASSERT(Valid(texture));

  if (texture->free_function && texture->data.has_value()) {
    texture->free_function(texture->data.value);
    texture->data.clear();
  }
}

const char* ToString(TextureType type) {
  switch (type) {
    case TextureType::kOpenGL: return "OpenGL";
    case TextureType::kVulkan: return "Vulkan";
    case TextureType::kLast: return "Last";
  }

  NOT_REACHED("Invalid texture type.");
  return nullptr;

}

}  // namespace warhol
