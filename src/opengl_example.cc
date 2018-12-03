// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.


// TODO(Cristian): Write own assert.
#include <assert.h>

#include <math.h>
#include <stdio.h>

#include <iostream>

#include <third_party/imgui/imgui.h>
#include <third_party/stb/stb_image.h>

#include "src/assets.h"
#include "src/camera.h"
#include "src/debug/time_logger.h"
#include "src/debug/timer.h"
#include "src/imgui/imgui_context.h"
#include "src/input/input.h"
#include "src/graphics/GL/def.h"
#include "src/graphics/GL/utils.h"
#include "src/platform/platform.h"
#include "src/math/math.h"
#include "src/model/cube.h"
#include "src/model/plane.h"
#include "src/sdl2/sdl_context.h"
#include "src/sdl2/def.h"
#include "src/shader.h"
#include "src/texture.h"
#include "src/texture_atlas.h"
#include "src/texture_array.h"
#include "src/utils/coords.h"
#include "src/utils/file.h"
#include "src/utils/log.h"
#include "src/utils/glm_impl.h"
#include "src/utils/macros.h"
#include "src/voxel_terrain.h"

/**
 * Simple OpenGL experiments to understand how to build a renderer.
 *
 * TODOs:
 *
 * - Remove all calls to CHECK_GL_ERRORS and use GL_CALL instead.
 * - Use GLHandle on all elements.
 * - Used static indices to attributes (Not we have Shader::Attributes::kModel
 *   as a name, but it should also have the attribute number).
 * - Use UBO (Uniform Buffer Objects) to bind common uniforms around.
 * - Move math into utils. (The dependencies are not good between these).
 * - Don't hardcode VoxelChunks to cubes.
 * - Create imgui_def.h and pass it through the context too.
 * - Create const char* constants for shader uniforms and attributes.
 * - Find out about sRGB OpenGL extensions (Handmade Hero).
 * - Make ReadWholeFile return a string instead of vector of char.
 * - Replace glm with my own math library (or find a decent one online). The
 *   API is too awkward, both header-wise and specially getting the pointers to
 *   the raw data (glm::value_ptr()... really?).
 * - Have the shader receive the camera and set the projection/view matrices
 *   and not the other way around as it is now.
 * - Find out a way to better handle assets instead of just having them
 *   committed to github.
 * - Move EventAction outside of SDL, as we could use it with another window
 *   library and it should just work (tm).
 * - Handle mouse buttons similar as keyboard (ask for down and up).
 * - Make a scene graph.
 *
 * WAAAY DOWN THE LINE:
 * - Abstract the renderer and enable Vulkan (that way we don't depend on the
 *   crappy OpenGL drivers).
 */

using namespace warhol;

int main() {
  SDLContext sdl_context;
  if (!sdl_context.Init())
    return 1;

  GL::Init();
  SDL_GL_SetSwapInterval(1);  // Enable v-sync.

  // Test current executable path.
  LOG(INFO) << "Current executable: " << Platform::GetCurrentExecutablePath();

  // Data about displays.
  LOG(INFO) << "Information from SDL:" << std::endl
            << "Amount of displays: " << SDL_GetNumVideoDisplays();

  // Test OpenGL is running.
  LOG(INFO) << std::endl
            << "OpenGL Vendor: " << glGetString(GL_VENDOR) << std::endl
            << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl
            << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl
            << "OpenGL Shading Language Version: "
            << glGetString(GL_SHADING_LANGUAGE_VERSION);

  GL_CALL(glEnable, GL_DEPTH_TEST);

  // ImGUI ---------------------------------------------------------------------

  ImguiContext imgui_context;
  if (!imgui_context.Init()) {
    LOG(ERROR) << "Could not initialize ImguiContext";
    exit(1);
  }

 // Shaders -------------------------------------------------------------------

  Shader simple_shader =
      Shader::FromAssetPath("simple", "simple.vert", "simple.frag");
  assert(simple_shader.valid());

  Shader alpha_test_shader =
      Shader::FromAssetPath("alpha_test", "simple.vert", "alpha_test.frag");
  assert(alpha_test_shader.valid());

  Shader voxel_shader =
      Shader::FromAssetPath("voxel", "simple.vert", "voxel.frag");
  assert(voxel_shader.valid());

  Shader one_texture =
      Shader::FromAssetPath("one_texture", "simple.vert", "one_tex.frag");
  assert(one_texture.valid());

  // Cube "model" --------------------------------------------------------------

  const auto& cube_vertices = Cube::GetVertices();

  // Generate the VAO that will hold the configuration.
  uint32_t cube_vao;
  GL_CALL(glGenVertexArrays, 1, &cube_vao);
  GL_CALL(glBindVertexArray, cube_vao);

  // Create cube VBO
  uint32_t cube_vbo;
  GL_CALL(glGenBuffers, 1, &cube_vbo);
  GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, cube_vbo);
  GL_CALL(glBufferData, GL_ARRAY_BUFFER,
                        sizeof(float) * cube_vertices.size(),
                        cube_vertices.data(),
                        GL_STATIC_DRAW);
  // How to interpret the buffer
  GLsizei cube_stride = (GLsizei)(5 * sizeof(float));
  auto a_pos = simple_shader.GetAttribute(Shader::Attribute::kPos);
  if (!a_pos)
    return 1;
  GL_CALL(glVertexAttribPointer,
          a_pos->location, 3, GL_FLOAT, GL_FALSE, cube_stride, (void*)0);
  GL_CALL(glEnableVertexAttribArray, a_pos->location);

  auto a_tex_coord0 = simple_shader.GetAttribute(Shader::Attribute::kTexCoord0);
  if (!a_tex_coord0)
    return 1;
  GL_CALL(glVertexAttribPointer, a_tex_coord0->location, 2,
          GL_FLOAT, GL_FALSE, cube_stride,
          (void*)(3 * sizeof(float)));
  GL_CALL(glEnableVertexAttribArray, a_tex_coord0->location);

  auto a_tex_coord1 = simple_shader.GetAttribute(Shader::Attribute::kTexCoord1);
  if (!a_tex_coord1)
    return 1;
  GL_CALL(glVertexAttribPointer, a_tex_coord1->location,
                                 2, GL_FLOAT, GL_FALSE, cube_stride,
                                 (void*)(3 * sizeof(float)));
  GL_CALL(glEnableVertexAttribArray, a_tex_coord1->location);

  GL_CALL(glBindVertexArray, NULL);

  // Plane "model" -------------------------------------------------------------

  auto plane_vertices = Plane::Create(10.0f, 10.0f);

  uint32_t plane_vao;
  GL_CALL(glGenVertexArrays, 1, &plane_vao);
  GL_CALL(glBindVertexArray, plane_vao);

  uint32_t plane_vbo;
  GL_CALL(glGenBuffers, 1, &plane_vbo);
  GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, plane_vbo);

  // Send the vertices over.
  GL_CALL(glBufferData, GL_ARRAY_BUFFER,
                        sizeof(float) * plane_vertices.size(),
                        plane_vertices.data(),
                        GL_STATIC_DRAW);

  // How to interpret the plane.
  GLsizei plane_stride = (GLsizei)(5 * sizeof(float));
  GL_CALL(glVertexAttribPointer,
          0, 3, GL_FLOAT, GL_FALSE, plane_stride, (void*)0);
  GL_CALL(glEnableVertexAttribArray, 0);
  GL_CALL(glVertexAttribPointer,
          1, 2, GL_FLOAT, GL_FALSE, plane_stride, (void*)(3 * sizeof(float)));
  GL_CALL(glEnableVertexAttribArray, 1);
  GL_CALL(glVertexAttribPointer,
          2, 2, GL_FLOAT, GL_FALSE, plane_stride, (void*)(3 * sizeof(float)));
  GL_CALL(glEnableVertexAttribArray, 2);

  GL_CALL(glBindVertexArray, NULL);

  // Textures ------------------------------------------------------------------

  // Generate the textures.
  Texture wall(Assets::TexturePath("wall.jpg"));
  assert(wall.valid());
  Texture face(Assets::TexturePath("awesomeface.png"));
  assert(face.valid());
  Texture grid(Assets::TexturePath("grid.png"));
  assert(grid.valid());

  // Texture array -------------------------------------------------------------


  constexpr int side_count = 16;

  TextureArray2D tex_array({side_count, side_count, side_count * side_count},
                           side_count, GL_RGBA);
  if (!tex_array.Init())
    return 1;

  uint32_t face_vao;
  GL_CALL(glGenVertexArrays, 1, &face_vao);
  GL_CALL(glBindVertexArray, face_vao);

  uint32_t face_vbo = 0;
  float face_verts[] = {
    0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 1.0f,

    0.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,

    0.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
  };

  GL_CALL(glGenBuffers, 1, &face_vbo);
  GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, face_vbo);
  GL_CALL(glBufferData, GL_ARRAY_BUFFER, sizeof(face_verts), face_verts, GL_STATIC_DRAW);

  GL_CALL(glVertexAttribPointer, 0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
  GL_CALL(glEnableVertexAttribArray, 0);
  GL_CALL(glVertexAttribPointer, 1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)(sizeof(float) * 3 * 6));
  GL_CALL(glEnableVertexAttribArray, 1);
  GL_CALL(glVertexAttribPointer, 2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)(sizeof(float) * (3 * 6 + 2 * 6)));
  GL_CALL(glEnableVertexAttribArray, 2);

  GL_CALL(glBindVertexArray, NULL);

  std::vector<uint32_t> cut_textures(side_count * side_count);
  GL_CALL(glGenTextures, cut_textures.size(), cut_textures.data());

  LOG(DEBUG) << "CUT TEXTURE SIZE: " << cut_textures.size();
  for (auto i : cut_textures)
    printf("%d, ", i);
  printf("\n");
  fflush(stdout);

  // So that this matches what OpenGL expects.
  stbi_set_flip_vertically_on_load(true);
  int x, y, channels;
  uint8_t* data = stbi_load(Assets::TexturePath("atlas.png").data(),
                            &x, &y, &channels, 0);
  (void)data;
  LOG(DEBUG) << "X: " << x << ", Y: " << y << ", CHANNELS: " << channels;
  // Grid is square.
  assert(x == y);
  int elem_side = x / side_count;
  LOG(DEBUG) << "X: " << x << ", SIDE: " << elem_side;

  for (int i = 0; i < tex_array.size().z; i++) {
    /* uint32_t* ptr = (uint32_t*)elem_data.data(); */
    /* for (int j = 0; j < elem_side * elem_side; j += 1) { */
    /*   *ptr++ = 0xFF0000FFu; */
    /* } */


    std::vector<uint8_t> elem_data(elem_side * elem_side * 4);
    uint32_t* ptr = (uint32_t*)elem_data.data();
    auto coord = ArrayIndexToCoord2(side_count, i);
    LOG(DEBUG) << "Coord: " << coord.ToString();
    for (int v = 0; v < elem_side; v++) {
      uint32_t* base = (uint32_t*)data +
            x * elem_side * coord.y +   // Right initial Y .
            x * v +                     // Offset into the square.
            elem_side * coord.x;        // Initial X for square.
      for (int u = 0; u < elem_side; u++) {
        *ptr++ = *base++;
      }
    }

    GL_CALL(glBindTexture, GL_TEXTURE_2D, cut_textures[i]);
    GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    GL_CALL(glTexImage2D, GL_TEXTURE_2D, 0, GL_RGBA, elem_side, elem_side, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, elem_data.data());
  }


  // Matrices ------------------------------------------------------------------

  LOG(INFO) << "Window size. WIDTH: " << sdl_context.width()
            << ", HEIGHT: " << sdl_context.height();

  // Voxel Terrain -------------------------------------------------------------

  Texture atlas_texture(Assets::TexturePath("atlas.png"));
  TextureAtlas atlas(std::move(atlas_texture), 16, 16);

  VoxelTerrain terrain(&atlas);
  if (!terrain.Init()) {
    LOG(ERROR) << "Could not initialize voxel terrain";
    exit(1);
  }

  // Camera --------------------------------------------------------------------

  float camera_speed = 5.0f;
  Camera camera(&sdl_context, {-10.0f, 5.0f, 10.0f});
  camera.SetTarget({});
  camera.UpdateView();

  // Game loop -----------------------------------------------------------------

  InputState input = InputState::Create();

  struct UIState {
    bool polygon_mode = false;
    float interpolation = 0.0f;

  };
  UIState ui_state = {};

  LOG(INFO) << "Going to draw";

  bool running = true;
  while (running) {
    Timer frame_timer = Timer::ManualTimer();
    frame_timer.Init();

    SDLContext::EventAction action = sdl_context.NewFrame(&input);
    if (action == SDLContext::EventAction::kQuit)
      break;

    imgui_context.NewFrame(sdl_context, &input);

    if (input.keys_up[GET_KEY(Escape)])
      break;

    bool camera_changed = false;
    if (!imgui_context.keyboard_captured()) {
      auto prev_pos = camera.pos;
      if (input.up) {
        camera.pos +=
            camera.direction() * camera_speed * sdl_context.frame_delta();
      }
      if (input.down) {
        camera.pos -=
            camera.direction() * camera_speed * sdl_context.frame_delta();
      }
      if (input.left) {
        camera.pos -= camera.direction().cross(camera.up()) * camera_speed *
                      sdl_context.frame_delta();
      }
      if (input.right) {
        camera.pos += camera.direction().cross(camera.up()) * camera_speed *
                      sdl_context.frame_delta();
      }
      camera_changed = prev_pos != camera.pos;
    }

    float mouse_sensibility = 0.007f;
    constexpr float max_yaw = 89.0f;

    if (!imgui_context.mouse_captured()) {
      if (input.mouse_offset != Pair<int>{0, 0}) {
        if (input.mouse.right) {
          camera.yaw() += input.mouse_offset.x * mouse_sensibility;

          camera.pitch() -= input.mouse_offset.y * mouse_sensibility;
          if (camera.pitch() > max_yaw) {
            camera.pitch() = max_yaw;
          }
          if (camera.pitch() > 360.0f - max_yaw) {
            camera.pitch() = 360.0f - max_yaw;
          }

          /* camera.DirectionFromEuler(); */
          camera.SetDirection(DirectionFromEuler(camera.pitch(), camera.yaw()));
          camera_changed = true;
        }
      }

      if (input.mouse.wheel.y != 0) {
        camera.fov -= input.mouse.wheel.y;
        if (camera.fov < 1.0f)
          camera.fov = 1.0f;
        if (camera.fov > 45.0f)
          camera.fov = 45.0f;
        camera.UpdateProjection();
      }
    }

    if (camera_changed)
      camera.UpdateView();


    if (!ui_state.polygon_mode) {
      GL_CALL(glPolygonMode, GL_FRONT_AND_BACK, GL_FILL);
    } else {
      GL_CALL(glPolygonMode, GL_FRONT_AND_BACK, GL_LINE);
    }

    // Draw the triangle.
    GL_CALL(glClearColor, 0.137f, 0.152f, 0.637f, 1.00f);
    GL_CALL(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    simple_shader.Use();
    camera.SetProjection(&simple_shader);
    camera.SetView(&simple_shader);

    // Draw the cubes.
    GL_CALL(glBindVertexArray, cube_vao);
    // Set the cube textures.
		wall.Set(&simple_shader, GL_TEXTURE0);
    face.Set(&simple_shader, GL_TEXTURE1);
    simple_shader.SetMat4(Shader::Uniform::kModel,
                          glm::translate(glm::mat4(1.0f), {5.0f, 0.0, 0.0f}));
    GL_CALL(glDrawArrays, GL_TRIANGLES, 0, 36);

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

    /* for (size_t i = 0; i < ARRAY_SIZE(cube_positions); i++) { */
    /*   glm::mat4 model = glm::translate(glm::mat4(1.0f), cube_positions[i]); */
    /*   float angle = sdl_context.seconds() * glm::radians(20.0f * i); */
    /*   model = */
    /*       glm::rotate(model, angle, glm::vec3(1.0f, 0.3f, 0.5f)); */
    /*   simple_shader.SetMat4(Shader::Uniform::kModel, model); */

    /*   /1* glDrawArrays(GL_TRIANGLES, 0, 36); *1/ */
    /* } */

    GL_CALL(glActiveTexture, GL_TEXTURE0);
    simple_shader.SetFloat({"u_interpolation"}, ui_state.interpolation);

    /* one_texture.Use(); */
    GL_CALL(glBindVertexArray, face_vao);

    for (size_t i = 0; i < cut_textures.size(); i++) {
      auto coord = ArrayIndexToCoord2(side_count, i);
      /* LOG(DEBUG) << "INDEX: " << i << ", COORD: " << coord.ToString(); */
      auto model =
          glm::translate(glm::mat4(1.0f),
              glm::vec3(0.0f, coord.y * 1.1f, coord.x * 1.1f));


      simple_shader.SetInt(Shader::Uniform::kTexSampler0, 0);

      GL_CALL(glBindTexture, GL_TEXTURE_2D, cut_textures[i]);
      simple_shader.SetMat4(Shader::Uniform::kModel, model);
      glDrawArrays(GL_TRIANGLES, 0, 6);
    }



    // We only need one texture for the plane.
    grid.Set(&simple_shader, GL_TEXTURE0);

    // Draw the plane.
    alpha_test_shader.Use();
    camera.SetProjection(&alpha_test_shader);
    camera.SetView(&alpha_test_shader);

    GL_CALL(glBindVertexArray, plane_vao);
    // The model at the origin.
    simple_shader.SetMat4(Shader::Uniform::kModel, glm::mat4(1.0f));
    GL_CALL(glDrawArrays, GL_TRIANGLE_STRIP, 0, 4);


    /* voxel_shader.Use(); */
    /* camera.SetProjection(&voxel_shader); */
    /* camera.SetView(&voxel_shader); */
    /* terrain.Render(&voxel_shader); */

    float frame_time = frame_timer.End();

    // ImGUI
    ImGui::ShowDemoWindow(nullptr);

    {
      ImGui::Begin("Test window");

      ImGui::Text("Application Frame: %.3f ms", frame_time);
      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                  1000.0 * sdl_context.frame_delta(),
                  sdl_context.framerate());

      ImGui::InputFloat3("Camera direction", (float*)camera.direction().data());
      ImGui::InputFloat3("Angles (rad)", (float*)camera.rotation().data());
      float deg[3] = {rad2deg(camera.rotation().x),
                      rad2deg(camera.rotation().y),
                      rad2deg(camera.rotation().z)};

       if (ImGui::InputFloat3("Angles (deg)",
                             deg,
                             "%.3f",
                             ImGuiInputTextFlags_EnterReturnsTrue)) {
        camera.SetDirectionFromEuler(deg2rad(deg[0]), deg2rad(deg[1]));
        camera.UpdateView();
      }

      ImGui::LabelText("FOV", "%.3f", camera.fov);
      ImGui::LabelText("Seconds", "%.3f", sdl_context.seconds());

      ImGui::Separator();
      ImGui::Checkbox("Polygon mode", &ui_state.polygon_mode);
      ImGui::DragFloat("Interpolation", &ui_state.interpolation, 0.1f, 0.0f, 1.0f);

      ImGui::End();
    }
    imgui_context.Render();

    TimeLoggerManager::Get().LogFrame();


    SDL_GL_SwapWindow(sdl_context.get_window());
  }

  sdl_context.Clear();
  SDL_Quit();

  return 0;
}
