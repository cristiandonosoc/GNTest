// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <stdio.h>

/* #include <GL/gl3w.h> */

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "utils/file.h"
#include "utils/log.h"

using namespace warhol;

int main() {
  // Setup SDL2.
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
    printf("Error loading SDL: %s\n", SDL_GetError());
    return 1;
  }

  // Data about displays.
  printf("Information from SDL\n");
  printf("Amount of displays: %d\n", SDL_GetNumVideoDisplays());

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  SDL_Window* window =
      SDL_CreateWindow("Warhol",
                       SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       1280, 720,
                       SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

  std::vector<char> vertex_shader;
  Status res = ReadWholeFile("shaders/simple.vert", &vertex_shader);
  if (!res.ok()) {
    LOG(ERROR) << "Reading vertex shader: " << res.err_msg();
    return 1;
  }

  std::vector<char> fragment_shader;
  res = ReadWholeFile("shaders/simple.frag", &fragment_shader);
  if (!res.ok()) {
    LOG(ERROR) << "Reading fragment shader: " << res.err_msg();
    return 1;
  }

  LOG(INFO) << "Correctly read vertex shader: " << std::endl
    << fragment_shader.data();


  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
