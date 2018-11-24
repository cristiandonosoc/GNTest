// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include "src/utils/clear_on_move.h"
#include "src/utils/macros.h"

namespace warhol {

#define NAMED_TIMER(name, description) \
  ::warhol::Timer name(__FILE__, __LINE__, description)

#define TIMER(description) \
  NAMED_TIMER(__timer##__LINE_##__, description)

#define FUNCTION_TIMER() TIMER(__PRETTY_FUNCTION__)

class Timer {
 public:
  Timer(const char* file, int line, const char* description);
  ~Timer();
  DELETE_COPY_AND_ASSIGN(Timer);
  DELETE_MOVE_AND_ASSIGN(Timer);

  void End();

 private:
  // ID at 0 means non-initialized or already ended.
  uint64_t id_ = 0;
};

}  // namespace warhol
