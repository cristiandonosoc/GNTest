// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/utils/location.h"

#include <assert.h>
#include <stdio.h>

#include <utility>
#include <vector>

#include "warhol/utils/assert.h"
#include "warhol/utils/log.h"

namespace warhol {
namespace {

LocationStack* GetPerThreadLocationStack() {
  thread_local LocationStack stack;
  return &stack;
}

}  // namespace

void PushLocation(Location location) {
  LocationStack* stack = GetPerThreadLocationStack();
  ASSERT(stack->size < (int)LocationStack::kMaxLocationStackSize - 1);
  stack->locations[stack->size++] = std::move(location);
}

void PopLocation() {
  LocationStack* stack = GetPerThreadLocationStack();
  ASSERT(stack->size > 0);
  stack->size--;
}

LocationStack* GetLocationStack() {
  return GetPerThreadLocationStack();
}

void PrintLocationStack(const LocationStack& stack) {
  for (int i = stack.size - 1; i >= 0; i--) {
    const Location& loc = stack.locations[i];
    printf("%.2d. %s [%s:%d]\n", stack.size - i - 1,
                                 loc.function,
                                 loc.file,
                                 loc.line);
  }
  fflush(stdout);
}

}  // namespace warhol
