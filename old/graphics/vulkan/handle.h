// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "warhol/graphics/vulkan/def.h"
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

  template <typename T>
  Handle(Context* context, T extra_handle, HandleType handle)
      : context_(context),
        extra_handle_((void*)extra_handle),
        handle_(handle) {}

  void Clear() {
    InternalClear();
    Reset();
  }

  void Set(Context* context, HandleType handle) {
    InternalClear();
    context_ = context;
    handle_ = handle;
  }

  template <typename T>
  void SetWithExtraHandle(Context* context, T extra_handle, HandleType handle) {
    InternalClear();
    context_ = context;
    extra_handle_ = (void*)extra_handle;
    handle_ = handle;
  }

  DELETE_COPY_AND_ASSIGN(Handle);

  Handle(Handle&& rhs) {
    Move(&rhs);
  }

  Handle& operator=(Handle&& rhs) {
    if (this != &rhs)
      Move(&rhs);
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

  Context* context() const { return context_; }
  void* extra_handle() const { return extra_handle_; }

 private:
  void Move(Handle* rhs) {
    Clear();
    context_ = rhs->context_;
    extra_handle_ = rhs->extra_handle_;
    handle_ = rhs->handle_;
    rhs->Reset();
  }

  // This function creates the corresponding freeing of the resource.
  // These are specialized on demand. If you get weird Handle<type> undefined
  // errors, you're probably missing an specialization.
  void InternalClear();

  void Reset() {
    context_ = nullptr;
    extra_handle_ = nullptr;
    handle_ = VK_NULL_HANDLE;
  }

  Context* context_ = nullptr;
  void* extra_handle_ = nullptr;
  HandleType handle_ = VK_NULL_HANDLE;
};

}  // namespace vulkan
}  // namespace warhol
