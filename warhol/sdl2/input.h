// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

struct SDL_KeyboardEvent;
struct SDL_MouseWheelEvent;

namespace warhol {

// Fills in the InputState with the data from SDL2.

struct InputState;

void HandleKeyUpEvent(const SDL_KeyboardEvent&, InputState*);
void HandleKeysDown(InputState*);
void HandleMouse(InputState*);
void HandleMouseWheelEvent(const SDL_MouseWheelEvent&, InputState*);

}  // namespace
