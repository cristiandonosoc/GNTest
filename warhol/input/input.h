// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include "warhol/math/vec.h"

namespace warhol {

#define GET_KEY(key) ((uint8_t)::warhol::Keys::k##key)

enum class Keys {
  kUp, kDown, kLeft, kRight,
  kA, kB, kC, kD, kE, kF, kG, kH, kI, kJ, kK, kL, kM, kN, kEnhe /* Ñ */, kO, kP,
  kQ, kR, kS, kT, kU, kV, kW, kX, kY, kZ,
  k0, k1, k2, k3, k4, k5, k6, k7, k8, k9,
  kPageUp, kPageDown, kHome, kEnd, kInsert, kDelete,
  kBackspace, kSpace, kEnter, kEscape,
  kTab, kCtrl, kAlt, kShift, kSuper /* windows key */,
  kLAST,  // Not a key, used to verify the input buffer size.
};

struct InputState {
  static constexpr uint8_t kInputSize = 128;
  bool keys_down[kInputSize];
  bool keys_up[kInputSize];

  // Actual control state.
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

}  // warhol