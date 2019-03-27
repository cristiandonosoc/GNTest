// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "warhol/window/sdl/def.h"

namespace warhol {

struct InputState;

namespace sdl {

void HandleKeysDown(InputState*);
void HandleKeyUpEvent(const SDL_KeyboardEvent& key_event, InputState* input);
void HandleMouse(InputState*);
void HandleMouseWheelEvent(const SDL_MouseWheelEvent&, InputState*);

}  // namespace sdl
}  // namespace warhol
