// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <stdio.h>

#include <GL/gl3w.h>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "sdl_context.h"
#include "shader.h"
#include "utils/file.h"
#include "utils/log.h"

using namespace warhol;

int main() {
  SDLContext sdl_context;
  Status res = sdl_context.Init();
  if (!res.ok()) {
    LOG_STATUS(res) << res;
    return 1;
  }
  gl3wInit();

  // Data about displays.
  LOG(DEBUG) << "Information from SDL:" << std::endl
             << "Amount of displays: " << SDL_GetNumVideoDisplays();


  // Test OpenGL is running.
  LOG(DEBUG) << std::endl
             << "OpenGL Vendor: " << glGetString(GL_VENDOR) << std::endl
             << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl
             << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl
             << "OpenGL Shading Language Version: "
             << glGetString(GL_SHADING_LANGUAGE_VERSION);

  int vert_attribs;
  glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &vert_attribs);
  LOG(DEBUG) << std::endl << "Max Vertex Attributes: " << vert_attribs;

  std::vector<char> vertex_shader;
  res = ReadWholeFile("shaders/simple.vert", &vertex_shader);
  if (!res.ok()) {
    LOG(ERROR) << res;
    return 1;
  }

  std::vector<char> fragment_shader;
  res = ReadWholeFile("shaders/simple.frag", &fragment_shader);
  if (!res.ok()) {
    LOG_STATUS(res);
    return 1;
  }
  LOG(INFO) << "Correctly read fragment shader: " << std::endl
            << fragment_shader.data();

  // Create a shader.
  Shader shader(vertex_shader.data(), fragment_shader.data());
  res = shader.Init();
  if (!res.ok()) {
    LOG_STATUS(res);
    return 1;
  }

  LOG(DEBUG) << "Successfully compiled a shader!";

  // Vertices example.
  float vertices[] = {
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
  };

  // Generate the VAO that will hold the configuration.
  uint32_t vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // Generate the vertices buffer object.
  uint32_t vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  // Send the vertex data over.
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  // Tell OpenGL how to interpret the buffer.
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  bool running = true;
  while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
      }
    }
    if (!running)
      break;

    // Draw the triangle.
    glClearColor(0.137f, 0.152f, 0.637f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader.Use();

    // Get a color and send the uniform.
    float ticks = SDL_GetTicks() / 1000;
    float green = sin(ticks) / 2.0f + 0.5f;
    const Uniform* uniform = shader.GetUniform("u_color");
    glUniform4f(uniform->location, 0.0f, green, 0.0f, 1.0f);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    SDL_GL_SwapWindow(sdl_context.window);

    SDL_Delay(10);
  }

  sdl_context.Clear();
  SDL_Quit();

  return 0;
}
