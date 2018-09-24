// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <stdio.h>

#include <GL/gl3w.h>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "shader.h"
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

  // Setup an OpenGL context.
  SDL_GLContext gl_context = SDL_GL_CreateContext(window);
  SDL_GL_SetSwapInterval(1);  // Enable v-sync.
  gl3wInit();

  // Test OpenGL is running.
  LOG(DEBUG) << std::endl
             << "OpenGL Vendor: " << glGetString(GL_VENDOR) << std::endl
             << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl
             << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl
             << "OpenGL Shading Language Version: "
             << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl
             << "OpenGL Extension: " << glGetString(GL_EXTENSIONS);

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
  LOG(INFO) << "Correctly read fragment shader: " << std::endl
    << fragment_shader.data();

  // Create a shader.
  Shader shader(vertex_shader.data(), fragment_shader.data());
  res = shader.Init();
  if (!res.ok()) {
    LOG(ERROR) << "Initializing shader: " << res.err_msg();
    return 1;
  }

  LOG(INFO) << "Successfully compiled a shader!";

  // TODO: Do RAII resouce cleaning.
  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
