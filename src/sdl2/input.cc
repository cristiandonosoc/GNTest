// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/sdl2/input.h"

#include "src/input/input.h"
#include "src/sdl2/def.h"

namespace warhol {

void
HandleKeyUp(const SDL_KeyboardEvent& key_event, InputState* input) {
  switch (key_event.keysym.scancode) {
    case SDL_SCANCODE_ESCAPE: input->keys_up[GET_KEY(Escape)] = true; break;
    default: break;
  }
}

#define SET_SDL_KEY(sdl_key, key)        \
  if (key_state[SDL_SCANCODE_##sdl_key]) \
    input->keys_down[GET_KEY(key)] = true;

void
HandleKeysDown(InputState* input) {
  const uint8_t* key_state = SDL_GetKeyboardState(0);
  SET_SDL_KEY(UP, Up);
  SET_SDL_KEY(DOWN, Down);
  SET_SDL_KEY(LEFT, Left);
  SET_SDL_KEY(RIGHT, Right);

  SET_SDL_KEY(A, A);
  SET_SDL_KEY(B, B);
  SET_SDL_KEY(C, C);
  SET_SDL_KEY(D, D);
  SET_SDL_KEY(E, E);
  SET_SDL_KEY(F, F);
  SET_SDL_KEY(G, G);
  SET_SDL_KEY(H, H);
  SET_SDL_KEY(I, I);
  SET_SDL_KEY(J, J);
  SET_SDL_KEY(K, K);
  SET_SDL_KEY(L, L);
  SET_SDL_KEY(M, M);
  SET_SDL_KEY(N, N);
  SET_SDL_KEY(O, O);
  SET_SDL_KEY(P, P);
  SET_SDL_KEY(Q, Q);
  SET_SDL_KEY(R, R);
  SET_SDL_KEY(S, S);
  SET_SDL_KEY(T, T);
  SET_SDL_KEY(U, U);
  SET_SDL_KEY(V, V);
  SET_SDL_KEY(W, W);
  SET_SDL_KEY(X, X);
  SET_SDL_KEY(Y, Y);
  SET_SDL_KEY(Z, Z);
  SET_SDL_KEY(0, 0);
  SET_SDL_KEY(1, 1);
  SET_SDL_KEY(2, 2);
  SET_SDL_KEY(3, 3);
  SET_SDL_KEY(4, 4);
  SET_SDL_KEY(5, 5);
  SET_SDL_KEY(6, 6);
  SET_SDL_KEY(7, 7);
  SET_SDL_KEY(8, 8);
  SET_SDL_KEY(9, 9);
  SET_SDL_KEY(PAGEUP, PageUp);
  SET_SDL_KEY(PAGEDOWN, PageDown);
  SET_SDL_KEY(HOME, Home);
  SET_SDL_KEY(END, End);
  SET_SDL_KEY(INSERT, Insert);
  SET_SDL_KEY(DELETE, Delete);
  SET_SDL_KEY(BACKSPACE, Backspace);
  SET_SDL_KEY(SPACE, Space);
  SET_SDL_KEY(RETURN, Enter);
  SET_SDL_KEY(ESCAPE, Escape);
  SET_SDL_KEY(TAB, Tab);

  auto mod_state = SDL_GetModState();
  input->keys_down[GET_KEY(Ctrl)] = mod_state & KMOD_CTRL;
  input->keys_down[GET_KEY(Alt)] = mod_state & KMOD_ALT;
  input->keys_down[GET_KEY(Shift)] = mod_state & KMOD_SHIFT;
  input->keys_down[GET_KEY(Super)] = mod_state & KMOD_GUI;

  // Set the control state
  // TODO(Cristian): This should not be here.
  if (input->keys_down[GET_KEY(Up)] || input->keys_down[GET_KEY(W)])
    input->up = true;
  if (input->keys_down[GET_KEY(Down)] || input->keys_down[GET_KEY(S)])
    input->down = true;
  if (input->keys_down[GET_KEY(Left)] || input->keys_down[GET_KEY(A)])
    input->left = true;
  if (input->keys_down[GET_KEY(Right)] || input->keys_down[GET_KEY(D)])
    input->right = true;
}

void HandleMouse(InputState* input) {
  auto mouse_state = SDL_GetMouseState(&input->mouse.pos.x,
                                       &input->mouse.pos.y);
  input->mouse.left = mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT);
  input->mouse.middle = mouse_state & SDL_BUTTON(SDL_BUTTON_MIDDLE);
  input->mouse.right = mouse_state & SDL_BUTTON(SDL_BUTTON_RIGHT);

  input->mouse_offset = input->mouse.pos - input->prev_mouse.pos;
}


}  // namespace warhol
