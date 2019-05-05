// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include "warhol/math/vec.h"
#include "warhol/utils/log.h"

namespace warhol {

#define GET_KEY(key) ((uint8_t)::warhol::Key::k##key)

enum class Key {
  kUp, kDown, kLeft, kRight,
  kA, kB, kC, kD, kE, kF, kG, kH, kI, kJ, kK, kL, kM, kN, kEnhe /* Ñ */, kO, kP,
  kQ, kR, kS, kT, kU, kV, kW, kX, kY, kZ,
  k0, k1, k2, k3, k4, k5, k6, k7, k8, k9,
  kPageUp, kPageDown, kHome, kEnd, kInsert, kDelete,
  kBackspace, kSpace, kEnter, kEscape,
  kTab, kCtrl, kAlt, kShift, kSuper /* windows key */,
  kLast,  // Not a key, used to verify the input buffer size.
};
const char* KeyToString(Key);

struct InputState {
  static constexpr uint8_t kInputSize = 128;
  static_assert((uint8_t)Key::kLast < InputState::kInputSize);

  bool down_last_frame[kInputSize];
  bool down_this_frame[kInputSize];
  static_assert(sizeof(down_last_frame) == sizeof(down_this_frame));

  // These are equal to down_this_frame[<ARROW KEY>] == true.
  bool up = false;
  bool down = false;
  bool left = false;
  bool right = false;

  struct MouseState {
    Pair<int> pos;
    bool left = false;
    bool middle = false;
    bool right = false;
    // x represents horizontal scrolling.
    // y represents vertical scrolling.
    Pair<int> wheel;
  };

  MouseState prev_mouse;
  MouseState mouse;
  Pair<int> mouse_offset;

  // API.
  static InputState Create();

  // Clears the current state of the frame and moves any state that needs to be
  // tracked inter-frame (eg. mouse positions).
  static void InitFrame(InputState*);
};

inline bool KeyDown(InputState* input, Key key) {
  int val = (int)key;
  ASSERT(val < (int)Key::kLast);
  return input->down_this_frame[val];
}

inline bool KeyDownThisFrame(InputState* input, Key key) {
  int val = (int)key;
  ASSERT(val < (int)Key::kLast);
  return input->down_this_frame[val] && !input->down_last_frame[val];
}

inline bool KeyUpThisFrame(InputState* input, Key key) {
  int val = (int)key;
  ASSERT(val < (int)Key::kLast);
  return !input->down_this_frame[val] && input->down_last_frame[val];
}

}  // warhol
