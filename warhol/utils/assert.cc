// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/utils/assert.h"

namespace warhol {

void AssertionFailed(Location loc, const char* condition) {
  printf("Assertion failed at %s (%s:%d):\n",
         loc.function,
         loc.file,
         loc.line);
  printf("\n    %s\n\n",
         condition);
  LocationStack* stack = GetLocationStack();
  printf("Printing contextual stack of size: %d\n", stack->size);
  PrintLocationStack(*stack);
  exit(1);
}



}  // namespace warhol
