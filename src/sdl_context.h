// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <memory>

#include "utils/macros.h"
#include "utils/status.h"

struct SDL_Window;

namespace warhol {

struct SDLContextImpl;

class SDLContext {
 public:
  SDLContext();
  ~SDLContext();

  // If init is not success, the instance should not be used and should be
  // destroyed.
  Status Init();
  void Clear();

  bool valid() const { return impl_ != nullptr; }

  // Can be null if !valid().
  SDL_Window* GetWindow() const;

  int width() const;
  int height() const;

  // Returns the seconds since Init() was called. This is a fractional number.
  float GetSeconds() const;

 private:
  std::unique_ptr<SDLContextImpl> impl_;

  DELETE_COPY_AND_ASSIGN(SDLContext);
};

}  // namespace warhol
