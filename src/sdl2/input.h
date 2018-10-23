// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

struct SDL_KeyboardEvent;

namespace warhol {

// Fills in the InputState with the data from SDL2.

struct InputState;

void HandleKeyUp(const SDL_KeyboardEvent&, InputState*);
void HandleKeysDown(InputState*);

}  // namespace
