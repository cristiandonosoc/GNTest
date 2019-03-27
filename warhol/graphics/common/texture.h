// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <string>

#include "warhol/utils/clear_on_move.h"

namespace warhol {

struct Texture {
  Texture() = default;
  ~Texture();   // RAII "semantics".
  DEFAULT_MOVE_AND_ASSIGN(Texture);

  std::string name;

  uint32_t uuid;
  int x;
  int y;
  int channels;

  std::string path;

  // Has to be deleted in a special way.
  ClearOnMove<uint8_t*> data;
};

inline bool Valid(Texture* texture) { return texture->uuid != 0; }
inline bool Loaded(Texture* texture) { return texture->data.has_value(); }

// Thread safe. Will advance the UUID;
uint64_t GetNextTextureUUID();

// Needed to do some pre-processing, like inverting axis for OpenGL.
enum class TextureType {
  kOpenGL,
  kVulkan,
  kLast,
};
const char* ToString(TextureType);

// Creates a new texture.
bool LoadTexture(const std::string& path, TextureType, Texture* texture);

// Will only remove the data.
void UnloadTexture(Texture*);

}  // namespace warhol
