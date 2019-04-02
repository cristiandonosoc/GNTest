// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <string>

#include "warhol/utils/clear_on_move.h"
#include "warhol/utils/macros.h"

namespace warhol {

// Needed to do some pre-processing, like inverting axis for OpenGL.
enum class TextureType {
  kOpenGL,
  kVulkan,
  kLast,
};
const char* ToString(TextureType);


struct Texture {
  RAII_CONSTRUCTORS(Texture);

  ClearOnMove<uint64_t> uuid;

  std::string name;

  TextureType type = TextureType::kLast;
  int x;
  int y;
  int channels;

  std::string path;

  // The type of the function this texture should use to free the data upon
  // shutdown. If null, means that the data memory lifetime is handled by
  // someone else.
  using FreeFunction = void(*)(void*);
  FreeFunction free_function = nullptr;

  // Has to be deleted in a special way.
  ClearOnMove<uint8_t*> data;
};

inline bool Valid(Texture* texture) { return texture->uuid.value != 0; }
inline bool Loaded(Texture* texture) { return texture->data.has_value(); }

// Thread safe. Will advance the UUID;
uint64_t GetNextTextureUUID();

// Creates a new texture.
bool LoadTexture(const std::string& path, TextureType, Texture* texture);

// Will only remove the data.
void UnloadTexture(Texture*);

}  // namespace warhol
