// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/sdl2/input.h"


#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "src/input/input.h"

namespace warhol {

void
HandleKeyUp(const SDL_KeyboardEvent& key_event, InputState* input) {
  switch (key_event.keysym.scancode) {
    case SDL_SCANCODE_ESCAPE: input->keys_up[GET_KEY(Escape)] = true; break;
    default: break;
  }
}

void
HandleKeysDown(InputState* input) {
  const uint8_t* key_state = SDL_GetKeyboardState(0);
  if (key_state[SDL_SCANCODE_UP]) {
    input->keys_down[GET_KEY(Up)] = true;
    input->up = true;
  }
  if (key_state[SDL_SCANCODE_DOWN]) {
    input->keys_down[GET_KEY(Down)] = true;
    input->down = true;
  }
  if (key_state[SDL_SCANCODE_LEFT]) {
    input->keys_down[GET_KEY(Left)] = true;
    input->left = true;
  }
  if (key_state[SDL_SCANCODE_RIGHT]) {
    input->keys_down[GET_KEY(Right)] = true;
    input->right = true;
  }

  if (key_state[SDL_SCANCODE_W]) {
    input->keys_down[GET_KEY(W)] = true;
    input->up = true;
  }
  if (key_state[SDL_SCANCODE_S]) {
    input->keys_down[GET_KEY(S)] = true;
    input->down = true;
  }
  if (key_state[SDL_SCANCODE_A]) {
    input->keys_down[GET_KEY(A)] = true;
    input->left = true;
  }
  if (key_state[SDL_SCANCODE_D]) {
    input->keys_down[GET_KEY(D)] = true;
    input->right = true;
  }
}

void HandleMouse(InputState* input) {
  auto mouse_state = SDL_GetMouseState(&input->cur_mouse.x,
                                       &input->cur_mouse.y);
  input->cur_mouse.left = mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT);
  input->cur_mouse.middle = mouse_state & SDL_BUTTON(SDL_BUTTON_MIDDLE);
  input->cur_mouse.right = mouse_state & SDL_BUTTON(SDL_BUTTON_RIGHT);
}


}  // namespace warhol
