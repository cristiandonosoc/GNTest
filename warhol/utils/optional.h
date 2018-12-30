// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>
#include <utility>

#include "warhol/utils/assert.h"

// This optional works exactly as std::optional except that it will actually
// clear the value as a move (which normal optional for some insane reason
// doesn't do).

template <typename T>
class Optional {
 public:

  // API -----------------------------------------------------------------------

   T& operator*() {
     ASSERT(has_value_);
     return value_.t_;
   }
   T* operator->() {
     ASSERT(has_value_);
     return &(value_.t_);
   }
   T& value() { return *(*this); }

   bool has_value() const {
     return has_value_;
   }
   operator bool() const { return has_value_; }

   void Clear() {
    if (has_value_)
      value_.t_.~T();
     has_value_ = false;
   }

  // Constructors --------------------------------------------------------------

  Optional() : has_value_(false) {}

  Optional(const T& t) : has_value_(true) {
    value_.t_ = t;
  }

  Optional(T&& t) : has_value_(true) {
    value_.t_ = std::move(t);
  }

  // Copy constructors.
  Optional(const Optional& rhs) : has_value_(rhs.has_value_) {
    if (has_value_)
      value_.t_ = rhs.value_.t_;
  }
  Optional& operator=(const Optional& rhs) {
    if (this != &rhs) {
      has_value_ = rhs.has_value_;
      if (has_value_)
        value_.t_ = rhs.value_.t_;
    }
    return *this;
  }

  // Move constructor.
  Optional(Optional&& rhs) : has_value_(rhs.has_value_) {
    if (rhs.has_value_)
      value_.t_ = std::move(rhs.value_.t_);
    rhs.has_value_ = false;
  }
  Optional& operator=(Optional&& rhs) {
    if (this != &rhs) {
      has_value_ = rhs.has_value_;
      if (has_value_)
        value_.t_ = std::move(rhs.value_.t_);
      rhs.has_value_ = false;
    }
    return *this;
  }

  ~Optional() {
    Clear();
  }

 private:
  bool has_value_ = false;
  union Value {
    Value() {}
    ~Value() {}

    uint8_t blob_[sizeof(T)] = {};
    T t_;
  } value_;
};
