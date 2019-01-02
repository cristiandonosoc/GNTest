// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <vulkan/vulkan.h>

#include "warhol/utils/macros.h"

namespace warhol {
namespace vulkan {

struct Context;

// Represents a vulkan resource that is movable and gets cleared on destruction
// and assignment.
template <typename HandleType>
struct Handle {
  Handle() = default;
  Handle(Context* context, HandleType handle)
      : context_(context), handle_(handle) {}

  void Set(Context* context, HandleType handle) {
    InternalClear();
    context_ = context;
    handle_ = handle;
  }

 /* Handle& operator=(HandleType handle) { */
  /*   InternalClear(); */
  /*   handle_ = handle; */
  /*   return *this; */
  /* } */

  DELETE_COPY_AND_ASSIGN(Handle);

  Handle(Handle&& rhs) : context_(rhs.context_), handle_(rhs.handle_) {
    rhs.Reset();
  }

  Handle& operator=(Handle&& rhs) {
    context_ = rhs.context_;
    handle_ = rhs.handle_;
    rhs.Reset();
    return *this;
  }

  ~Handle() {
    InternalClear();
  }

  explicit operator bool() const { return handle_ != VK_NULL_HANDLE; }
  bool has_value() const { return handle_ != VK_NULL_HANDLE; }

  HandleType& operator*() { return handle_; }
  HandleType* operator->() { return &handle_; }
  HandleType& value() { return handle_; }

 private:
  // This function creates the corresponding freeing of the resource.
  // These are specialized on demand. If you get weird Handle<type> undefined
  // errors, you're probably missing an specialization.
  void InternalClear();

  void Reset() {
    context_ = nullptr;
    handle_ = VK_NULL_HANDLE;
  }

  Context* context_ = nullptr;
  HandleType handle_ = VK_NULL_HANDLE;
};

}  // namespace vulkan
}  // namespace warhol
