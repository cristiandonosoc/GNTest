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
#include "src/camera.h"
#include "src/model/cube.h"
#include "src/model/plane.h"
#include "src/sdl_context.h"
#include "src/shader.h"
#include "src/texture.h"
#include "src/texture_atlas.h"
#include "src/utils/macros.h"
#include "src/utils/file.h"
#include "src/utils/log.h"

#include "src/utils/glm_impl.h"

#include "src/model/minecraft_cube.h"

/**
 * Simple OpenGL experiments to understand how to build a renderer.
 *
 * TODOs:
 *
 * - Stop using Status and start logging directly at the call site.
 *   This is what Status is doing anyway and we might just as well return a
 *   boolean.
 * - Change GLM importing to use our headers so we can ignore the annoying
 *   BEGIN/END_IGNORE_WARNINGS macros.
 * - Have the shader receive the camera and set the projection/view matrices
 *   and not the other way around as it is now.
 * - Find out a way to better handle assets instead of just having them
 *   committed to github.
 */

using namespace warhol;

struct ControlState {
  bool up = false;
  bool down = false;
  bool left = false;
  bool right = false;
  bool escape = false;
};

// Returns whether the program should still be running.
void HandleKeyUp(const SDL_KeyboardEvent&, ControlState*);
void HandleKeyboard(ControlState*);

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

  std::vector<char> alpha_test_frag;
  ReadWholeFile(Assets::ShaderPath("alpha_test.frag"), &alpha_test_frag);
  Shader alpha_test_shader(vertex_shader.data(), alpha_test_frag.data());

  res = alpha_test_shader.Init();
  if (!res.ok()) {
    LOG_STATUS(res);
    return 1;
  }

  // Cube "model" --------------------------------------------------------------

  const auto& cube_vertices = Cube::GetVertices();

  // Generate the VAO that will hold the configuration.
  uint32_t cube_vao;
  glGenVertexArrays(1, &cube_vao);
  glBindVertexArray(cube_vao);

  // Create cube VBO
  uint32_t cube_vbo;
  glGenBuffers(1, &cube_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(float) * cube_vertices.size(),
               cube_vertices.data(),
               GL_STATIC_DRAW);
  // How to interpret the buffer
  GLsizei cube_stride = (GLsizei)(5 * sizeof(float));
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, cube_stride, (void*)0);
  glEnableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glVertexAttribPointer(
      2, 2, GL_FLOAT, GL_FALSE, cube_stride, (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(2);

  glBindVertexArray(NULL);

  /* glm::vec3 cube_positions[] = {glm::vec3(0.0f, 0.0f, 0.0f), */
  /*                               glm::vec3(2.0f, 5.0f, -15.0f), */
  /*                               glm::vec3(-1.5f, -2.2f, -2.5f), */
  /*                               glm::vec3(-3.8f, -2.0f, -12.3f), */
  /*                               glm::vec3(2.4f, -0.4f, -3.5f), */
  /*                               glm::vec3(-1.7f, 3.0f, -7.5f), */
  /*                               glm::vec3(1.3f, -2.0f, -2.5f), */
  /*                               glm::vec3(1.5f, 2.0f, -2.5f), */
  /*                               glm::vec3(1.5f, 0.2f, -1.5f), */
  /*                               glm::vec3(-1.3f, 1.0f, -1.5f)}; */

  // Plane "model" -------------------------------------------------------------

  auto plane_vertices = Plane::Create(10.0f, 10.0f);

  uint32_t plane_vao;
  glGenVertexArrays(1, &plane_vao);
  glBindVertexArray(plane_vao);

  uint32_t plane_vbo;
  glGenBuffers(1, &plane_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, plane_vbo);

  // Send the vertices over.
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(float) * plane_vertices.size(),
               plane_vertices.data(),
               GL_STATIC_DRAW);

  // How to interpret the plane.
  GLsizei plane_stride = (GLsizei)(5 * sizeof(float));
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, plane_stride, (void*)0);
  glEnableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glVertexAttribPointer(
      2, 2, GL_FLOAT, GL_FALSE, plane_stride, (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(2);

  glBindVertexArray(NULL);

  // Minecraft Cube ------------------------------------------------------------

  Texture atlas_texture(Assets::TexturePath("atlas.png"));
  TextureAtlas atlas(std::move(atlas_texture), 16, 16);

  /* // The minecraft cube handles the VAO, VBO */
  MinecraftCube minecraft_cube(&atlas);
  minecraft_cube.Init();
  minecraft_cube.set_position({1.0f, 1.0f, 1.0f});

  // Textures ------------------------------------------------------------------

  // Generate the textures.
  Texture wall(Assets::TexturePath("wall.jpg"));
  assert(wall.valid());
  Texture face(Assets::TexturePath("awesomeface.png"));
  assert(face.valid());
  Texture grid(Assets::TexturePath("grid.png"));
  assert(grid.valid());

  // Matrices ------------------------------------------------------------------

  LOG(INFO) << "Window size. WIDTH: " << sdl_context.width()
            << ", HEIGHT: " << sdl_context.height();

  // Camera --------------------------------------------------------------------

  float camera_speed = 5.0f;
  Camera camera(&sdl_context, {1.0f, 5.0f, 10.0f});

  // Game loop -----------------------------------------------------------------

  // When the last frame started.
  float last_frame_time = 0.0f;
  // Time from previous frame start to the current.
  float time_delta = 0.0f;

  bool running = true;
  while (running) {
    float current_time = sdl_context.GetSeconds();
    time_delta = current_time - last_frame_time;
    last_frame_time = current_time;

    ControlState control = {};
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT:
          running = false;
          break;
        case SDL_KEYUP:
          HandleKeyUp(event.key, &control);
        default:
          break;
      }
    }

    HandleKeyboard(&control);

    if (!running || control.escape)
      break;

    if (control.up) {
      camera.pos -= camera.front() * camera_speed * time_delta;
    }
    if (control.down) {
      camera.pos += camera.front() * camera_speed * time_delta;
    }
    if (control.left) {
      camera.pos += glm::normalize(glm::cross(camera.front(), camera.up())) *
                    camera_speed * time_delta;
    }
    if (control.right) {
      camera.pos -= glm::normalize(glm::cross(camera.front(), camera.up())) *
                    camera_speed * time_delta;
    }

    camera.UpdateView();

    // Draw the triangle.
    glClearColor(0.137f, 0.152f, 0.637f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);





    // Draw the cubes.
    glBindVertexArray(cube_vao);
    shader.Use();
    camera.SetProjection(&shader);
    camera.SetView(&shader);

    // Set the cube textures.
		wall.Set(&shader, GL_TEXTURE0);
    face.Set(&shader, GL_TEXTURE1);

    /* float seconds = sdl_context.GetSeconds(); */
    /* for (size_t i = 0; i < ARRAY_SIZE(cube_positions); i++) { */
    /*   glm::mat4 model = glm::translate(glm::mat4(1.0f), cube_positions[i]); */
    /*   float angle = seconds * glm::radians(20.0f * i); */
    /*   model = */
    /*       glm::rotate(model, angle, glm::vec3(1.0f, 0.3f, 0.5f)); */
    /*   shader.SetMat4("model", model); */

    /*   glDrawArrays(GL_TRIANGLES, 0, 36); */
    /* } */


    // minecraft_cube.SetUniforms(&shader);
    minecraft_cube.Render(&shader);

    // We only need one texture for the plane.
    grid.Set(&shader, GL_TEXTURE0);
    Texture::Disable(GL_TEXTURE1);

    // Draw the plane.
    alpha_test_shader.Use();
    camera.SetProjection(&alpha_test_shader);
    camera.SetView(&alpha_test_shader);

    glBindVertexArray(plane_vao);
    // The model at the origin.
    shader.SetMat4("model", glm::mat4(1.0f));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


		/* glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); */
    /* glDrawArrays(GL_TRIANGLES, 0, 36); */

    SDL_GL_SwapWindow(sdl_context.GetWindow());

    SDL_Delay(10);
  }

  sdl_context.Clear();
  SDL_Quit();

  return 0;
}

void
HandleKeyUp(const SDL_KeyboardEvent& key_event, ControlState* state) {
  switch (key_event.keysym.scancode) {
    case SDL_SCANCODE_ESCAPE: state->escape = true; break;
    default: break;
  }
}

void
HandleKeyboard(ControlState* state) {
  const uint8_t* key_state = SDL_GetKeyboardState(0);

  if (key_state[SDL_SCANCODE_UP] || key_state[SDL_SCANCODE_W])
    state->up = true;
  if (key_state[SDL_SCANCODE_DOWN] || key_state[SDL_SCANCODE_S])
      state->down = true;
  if (key_state[SDL_SCANCODE_LEFT] || key_state[SDL_SCANCODE_A])
      state->left = true;
  if (key_state[SDL_SCANCODE_RIGHT] || key_state[SDL_SCANCODE_D])
    state->right = true;
}

