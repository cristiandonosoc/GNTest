// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <stdio.h>

#include <SDL2/SDL.h>

int main(int, char**) {
  // Setup SDL
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    printf("Error starting SDL: %s\n", SDL_GetError());
    return 1;
  }

  SDL_Window *window = SDL_CreateWindow("Example", SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);
  if (!window) {
    printf("Error creating window: %s\n", SDL_GetError());
    return 1;
  }

  SDL_Surface* surface = SDL_GetWindowSurface(window);
  SDL_Renderer* renderer = SDL_CreateSoftwareRenderer(surface);
  if (!renderer) {
    printf("Error creating renderer: %s\n", SDL_GetError());
    return 1;
  }

  SDL_SetRenderDrawColor(renderer, 0xAA, 0x20, 0x44, 0xFF);
  SDL_RenderClear(renderer);

  while (true) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT)
        break;

      if ((e.type == SDL_KEYDOWN) && (e.key.keysym.sym == SDLK_ESCAPE))
        break;
    }
  }

  SDL_Quit();
  return 0;
}
