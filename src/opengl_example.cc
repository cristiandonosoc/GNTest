// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <math.h>
#include <stdio.h>

#include <GL/gl3w.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <third_party/stb/stb_image.h>

#include "src/arch/arch_provider.h"
#include "src/assets.h"
#include "src/sdl_context.h"
#include "src/shader.h"
#include "src/utils/file.h"
#include "src/utils/log.h"

using namespace warhol;

// Returns whether the program should still be running.
bool HandleKeyUp(const SDL_KeyboardEvent&);
void GenerateTexture();


int main() {
  SDLContext sdl_context;
  Status res = sdl_context.Init();
  if (!res.ok()) {
    LOG_STATUS(res) << res;
    return 1;
  }
  gl3wInit();

  // Test current executable path.
  LOG(DEBUG) << "Current executable: "
             << arch::ArchProvider::GetCurrentExecutablePath();

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
  LOG(DEBUG) << "Max Vertex Attributes: " << vert_attribs;

  std::vector<char> vertex_shader;
  res = ReadWholeFile(Assets::ShaderPath("simple.vert"), &vertex_shader);
  if (!res.ok()) {
    LOG(ERROR) << res;
    return 1;
  }

  std::vector<char> fragment_shader;
  res = ReadWholeFile(Assets::ShaderPath("simple.frag"), &fragment_shader);
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

  float vertices[] = {
    // positions         // colors
     0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // bottom right
    -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // bottom left
     0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f    // top
  };

  /* float tex_coords[] = { */
  /*   0.0f, 0.0f,  // lower-left corner */
  /*   1.0f, 0.0f,  // lower-right corner */
  /*   0.5f, 1.0f   // top-center corner */
  /* }; */



  // Generate the VAO that will hold the configuration.
  uint32_t vao;
  glGenVertexArrays(1, &vao);

  // Generate the vertices buffer object.
  uint32_t vbo;
  glGenBuffers(1, &vbo);

  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  // Send the vertex data over.
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  // Tell OpenGL how to interpret the buffer.
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                        6 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  shader.Use();


    // Get a color and send the uniform.
    /* float ticks = (float)SDL_GetTicks() / 1000.0f; */
    /* float green = sin(ticks) / 2.0f + 0.5f; */
    const Uniform* uniform = shader.GetUniform("u_color");
    if (!uniform) {
      LOG(WARNING) << "Could not find uniform \"u_color\"";
      /* glUniform4f(uniform->location, 0.0f, green, 0.0f, 1.0f); */
    }



  bool running = true;
  while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT:
          running = false;
          break;
        case SDL_KEYUP:
          running = HandleKeyUp(event.key);
        default:
          break;
      }
    }
    if (!running)
      break;

    // Draw the triangle.
    glClearColor(0.137f, 0.152f, 0.637f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* // Get a color and send the uniform. */
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float green = sin(ticks) / 2.0f + 0.5f;
    /* const Uniform* uniform = shader.GetUniform("u_color"); */
    /* glUniform4f(uniform->location, 0.0f, green, 0.0f, 1.0f); */


    shader.SetFloat("u_x_offset", green);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    SDL_GL_SwapWindow(sdl_context.window);

    SDL_Delay(10);
  }

  sdl_context.Clear();
  SDL_Quit();

  return 0;
}

bool HandleKeyUp(const SDL_KeyboardEvent& key_event) {
  switch (key_event.keysym.scancode) {
    case SDL_SCANCODE_ESCAPE:
      return false;
    default:
      return true;
  }
}


/* void GenerateTexture() { */
/*   int width, height; */

/*   uint8_t* data = stbi_load( */

/* } */
