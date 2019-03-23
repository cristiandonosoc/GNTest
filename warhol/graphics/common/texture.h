// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <memory>
#include <string>

namespace warhol {

struct Texture {
  uint32_t uuid;
  uint32_t x;
  uint32_t y;
  uint32_t channels;

  std::string path;

  std::unique_ptr<uint8_t[]> data;
};

inline bool Valid(Texture* texture) { return !!texture->data; }

uint32_t GetNextTextureUUID();

// Creates a new texture.
bool LoadTexture(const std::string& path, Texture* texture);

// Will only remove the data.
void UnloadTexture(Texture*);

}  // namespace warhol
