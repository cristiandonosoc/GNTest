// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdio.h>

#include "utils/macros.h"

template <typename T, typename UniqueTraits>
struct UniqueHandle {
  UniqueHandle(HandleType handle) : handle(std::move(handle)) {}
  ~UniqueHandle() {
    UniqueTraits::Close(handle);
  }

  T handle;

  DELETE_COPY_AND_ASSIGN(UniqueHandle);
  DEFAULT_MOVE_AND_ASSIGN(UniqueHandle);
};

// File specialization.
struct FileTraits {
  static Close(FILE* fd) {
    if (fd)
      fclose(fd);
  }
}
