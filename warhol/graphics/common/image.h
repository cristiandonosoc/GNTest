// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <string>

#include "warhol/utils/clear_on_move.h"
#include "warhol/utils/macros.h"

namespace warhol {

// NOTE: Image assumes that data was loaded through stb_image.
struct Image {
  // Function to be used to free the data.
  using FreeFunction = void(*)(void*);

  enum class Type {
    k1D,
    k2D,
    k3D,
    kLast,
  };

  enum class Format {
    kRGBA8,
    kLast,
  };


  static Image Create2DImageFromPath(const std::string&);
  // Will also clear the fields from |this|.
  uint8_t* Release();

  bool valid() const { return data.value != nullptr; }

  Image();
  ~Image();
  DELETE_COPY_AND_ASSIGN(Image);
  DEFAULT_MOVE_AND_ASSIGN(Image);

  int width = -1;
  int height = -1;
  int channels = -1;

  ClearOnMove<uint8_t*> data = nullptr;
  int data_size = 0;
  FreeFunction free_function = nullptr;

  Type type = Type::kLast;
  Format format = Format::kLast;
};

}  // namespace warhol
