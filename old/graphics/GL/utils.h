// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>
#include <stdlib.h>

#include <utility>

#include "warhol/graphics/GL/def.h"
#include "warhol/utils/macros.h"

namespace warhol {

const char* GLEnumToString(GLenum);
size_t GLEnumToSize(GLenum);


template <GLenum Resource>
struct GLHandle {
  GLHandle() = default;
  GLHandle(uint32_t h) : handle(h) {}

  GLHandle& operator=(uint32_t h) {
    this->handle = h;
    return *this;
  }
  ~GLHandle() { Clear(); }
  DELETE_COPY_AND_ASSIGN(GLHandle);


  // Move.
  GLHandle(GLHandle&& other) {
    *this = std::move(other);
  }
  GLHandle& operator=(GLHandle&& other) {
    // They use the same storage, so either will work.
    handle = other.handle;
    other.handle = 0;
    return *this;
  }

  void Clear() {
    InternalClear();
    handle = 0;
  }

  explicit operator bool() const { return handle != 0; }
  uint32_t& operator*() { return handle; }

  uint32_t handle;

 private:
  // The internal clears are where each resource freeing strategy is handled.
  // These are specialized on-demand. If you get weird "GLHandle<32423>
  // undefined" errors, this is probably the droid you're looking for.
  void InternalClear();
};

}  // namespace warhol
