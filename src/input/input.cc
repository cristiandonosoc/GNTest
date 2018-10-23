// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/input/input.h"

#include <assert.h>

namespace warhol {

InputState InputState::Create() {
  InputState state;
  assert((uint8_t)Keys::kLAST < InputState::kInputSize);
  return state;

}

void InputState::Reset(InputState* input) {
  for (int i = 0; i < kInputSize; i++) {
    input->keys_up[i] = false;
    input->keys_down[i] = false;
  }

  // Actual control state.
  input->up = false;
  input->down = false;
  input->left = false;
  input->right = false;
}

}  // namespace warhol
