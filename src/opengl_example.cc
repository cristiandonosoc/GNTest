// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.


// TODO(Cristian): Write own assert.
#include <assert.h>

#include <math.h>
#include <stdio.h>

#include <iostream>

#include <GL/gl3w.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <third_party/stb/stb_image.h>

#include "src/arch/arch_provider.h"
#include "src/assets.h"
#include "src/model/cube.h"
#include "src/sdl_context.h"
#include "src/shader.h"
#include "src/texture.h"
#include "src/utils/macros.h"
#include "src/utils/file.h"
#include "src/utils/log.h"

BEGIN_IGNORE_WARNINGS()
#include <third_party/include/glm/glm.hpp>
#include <third_party/include/glm/gtc/matrix_transform.hpp>
#include <third_party/include/glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
END_IGNORE_WARNINGS()

using namespace warhol;

// Returns whether the program should still be running.
bool HandleKeyUp(const SDL_KeyboardEvent&);
void GenerateTextures();


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

  glEnable(GL_DEPTH_TEST);

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

  // Generate the VAO that will hold the configuration.
  uint32_t vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // Plane "model" -------------------------------------------------------------

  float vertices[] = {
	   // positions         // colors           // texture coords
	   0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
	   0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
	  -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
	  -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left
	};

	unsigned int indices[] = {  // note that we start from 0!
    0, 1, 3,   // first triangle
    1, 2, 3    // second triangle
	};

  // Generate the vertices buffer object.
  uint32_t vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  // Send the vertex data over.
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  // Tell OpenGL how to interpret the buffer
  GLsizei stride = 8 * sizeof(float);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
												(void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride,
												(void*)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);
  // Indices
  uint32_t ebo;
  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
							 GL_STATIC_DRAW);

  // Cube "model" --------------------------------------------------------------

  const auto& cube_vertices = Cube::GetVertices();

  // Create cube VBO
  uint32_t cube_vbo;
  glGenBuffers(1, &cube_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(float) * cube_vertices.size(),
               cube_vertices.data(),
               GL_STATIC_DRAW);
  // How to interpret the buffer
  stride = 5 * sizeof(float);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
  glEnableVertexAttribArray(0);
  /* glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, */
												/* (void*)(3 * sizeof(float))); */
  glDisableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride,
												(void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(2);
  shader.Use();

  // Textures ------------------------------------------------------------------

  // Generate the textures.
  Texture wall(Assets::TexturePath("wall.jpg"));
  LOG(DEBUG) << "Wall channels: " << wall.channels();
  assert(wall.valid());
  Texture face(Assets::TexturePath("awesomeface.png"));
  assert(face.valid());
  LOG(DEBUG) << "Face channels: " << face.channels();


  // Matrices ------------------------------------------------------------------

  // TODO(Cristian): Move this to SDLContext.
  int width, height;
  SDL_GetWindowSize(sdl_context.window, &width, &height);
  LOG(INFO) << "Window size. WIDTH: " << width << ", HEIGHT: " << height;

  // These are static for now.
  glm::mat4 view =
      glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
  shader.SetMatrix("view", 4, glm::value_ptr(view));

  glm::mat4 projection = glm::perspective(glm::radians(45.0f),
                                          (float)width / (float)height,
                                          0.1f, 100.0f);
  shader.SetMatrix("projection", 4, glm::value_ptr(projection));

  glm::vec3 cube_positions[] = {glm::vec3(0.0f, 0.0f, 0.0f),
                                glm::vec3(2.0f, 5.0f, -15.0f),
                                glm::vec3(-1.5f, -2.2f, -2.5f),
                                glm::vec3(-3.8f, -2.0f, -12.3f),
                                glm::vec3(2.4f, -0.4f, -3.5f),
                                glm::vec3(-1.7f, 3.0f, -7.5f),
                                glm::vec3(1.3f, -2.0f, -2.5f),
                                glm::vec3(1.5f, 2.0f, -2.5f),
                                glm::vec3(1.5f, 0.2f, -1.5f),
                                glm::vec3(-1.3f, 1.0f, -1.5f)};

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

		wall.Use(shader, GL_TEXTURE0);
    face.Use(shader, GL_TEXTURE1);

    glBindVertexArray(vao);

    for (size_t i = 0; i < ARRAY_SIZE(cube_positions); i++) {
      glm::mat4 model = glm::translate(glm::mat4(1.0f), cube_positions[i]);
      float angle = (float)SDL_GetTicks() / 1000.0f * glm::radians(20.0f * i);
      model =
          glm::rotate(model, angle, glm::vec3(1.0f, 0.3f, 0.5f));
      shader.SetMatrix("model", 4, glm::value_ptr(model));

      glDrawArrays(GL_TRIANGLES, 0, 36);
    }

		/* glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); */
    glDrawArrays(GL_TRIANGLES, 0, 36);

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

void GenerateTextures() {






}
