// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

namespace warhol {

#define GET_KEY(key) ((uint8_t)::warhol::Keys::k##key)

enum class Keys {
  kA, kB, kC, kD, kE, kF, kG, kH, kI, kJ, kK, kL, kM,
  kN, kÑ, kO, kP, kQ, kR, kS, kT, kU, kV, kW, kX, kY, kZ,

  k0, k1, k2, k3, k4, k5, k6, k7, k8, k9,

  kUp, kDown, kLeft, kRight,

  kEscape,
  kCtrl, kAlt, kShift,
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
    int x;   // In pixels.
    int y;   // In pixels.
    bool left = false;
    bool middle = false;
    bool right = false;
  };
  MouseState prev_mouse;
  MouseState cur_mouse;

  // API.
  static InputState Create();

  // Clears the current state of the frame and moves any state that needs to be
  // tracked inter-frame (eg. mouse positions).
  static void InitFrame(InputState*);
};

}  // warhol
