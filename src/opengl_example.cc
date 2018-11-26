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
#include "src/utils/macros.h"
#include "src/utils/file.h"
#include "src/utils/log.h"
#include "src/utils/glm_impl.h"
#include "src/voxel_terrain.h"

/**
 * Simple OpenGL experiments to understand how to build a renderer.
 *
 * TODOs:
 *
 * - Use GLHandle on all elements.
 * - Used static indices to attributes (Not we have Shader::Attributes::kModel
 *   as a name, but it should also have the attribute number).
 * - Use UBO (Uniform Buffer Objects) to bind common uniforms around.
 * - Move math into utils. (The dependencies are not good between these).
 * - Don't hardcode VoxelChunks to cubes.
 * - Create imgui_def.h and pass it through the context too.
 * - Create const char* constants for shader uniforms and attributes.
 * - Find out about sRGB OpenGL extensions (Handmade Hero).
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
  LOG(DEBUG) << "Current executable: " << Platform::GetCurrentExecutablePath();

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
  /* glEnable(GL_TEXTURE_2D); */

  if (CHECK_GL_ERRORS("GL enables"))
    exit(1);

  // ImGUI ---------------------------------------------------------------------

  ImguiContext imgui_context;
  if (!imgui_context.Init()) {
    LOG(ERROR) << "Could not initialize ImguiContext";
    exit(1);
  }

  // Shader Sources ------------------------------------------------------------

  int vert_attribs;
  glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &vert_attribs);
  LOG(DEBUG) << "Max Vertex Attributes: " << vert_attribs;

  std::vector<char> vertex_shader;
  if (!ReadWholeFile(Assets::ShaderPath("simple.vert"), &vertex_shader))
    return 1;

  std::vector<char> fragment_shader;
  if (!ReadWholeFile(Assets::ShaderPath("simple.frag"), &fragment_shader))
    return 1;

  std::vector<char> voxel_frag_src;
  if (!ReadWholeFile(Assets::ShaderPath("voxel.frag"), &voxel_frag_src))
    return 1;

  // Shaders -------------------------------------------------------------------

  // Simple shader.
  Shader shader(vertex_shader.data(), fragment_shader.data());
  if (!shader.Init())
    return 1;

  // Alpha test shader.
  std::vector<char> alpha_test_frag;
  ReadWholeFile(Assets::ShaderPath("alpha_test.frag"), &alpha_test_frag);
  Shader alpha_test_shader(vertex_shader.data(), alpha_test_frag.data());

  if (!alpha_test_shader.Init())
    return 1;

  // Voxel shader.
  Shader voxel_shader(vertex_shader.data(), voxel_frag_src.data());
  if (!voxel_shader.Init())
    return 1;

  if (CHECK_GL_ERRORS("Creating shaders"))
    exit(1);

  for (const auto& [key, attrib] : shader.attributes()) {
    LOG(DEBUG) << "Attribute " << key << ": " << attrib.location;
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
  auto a_pos = shader.GetAttribute("a_pos");
  glVertexAttribPointer(
      a_pos->location, 3, GL_FLOAT, GL_FALSE, cube_stride, (void*)0);
  glEnableVertexAttribArray(a_pos->location);

  auto a_tex_coord0 = shader.GetAttribute("a_tex_coord0");
  glVertexAttribPointer(a_tex_coord0->location,
                        2,
                        GL_FLOAT,
                        GL_FALSE,
                        cube_stride,
                        (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(a_tex_coord0->location);

  auto a_tex_coord1 = shader.GetAttribute("a_tex_coord1");
  glVertexAttribPointer(a_tex_coord1->location,
                        2,
                        GL_FLOAT,
                        GL_FALSE,
                        cube_stride,
                        (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(a_tex_coord1->location);

  glBindVertexArray(NULL);

  if (CHECK_GL_ERRORS("Creating cube"))
    exit(1);

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
  glVertexAttribPointer(
      1, 2, GL_FLOAT, GL_FALSE, plane_stride, (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(
      2, 2, GL_FLOAT, GL_FALSE, plane_stride, (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(2);

  glBindVertexArray(NULL);

  if (CHECK_GL_ERRORS("Creating plane"))
    return 1;

  // Textures ------------------------------------------------------------------

  // Generate the textures.
  Texture wall(Assets::TexturePath("wall.jpg"));
  assert(wall.valid());
  Texture face(Assets::TexturePath("awesomeface.png"));
  assert(face.valid());
  Texture grid(Assets::TexturePath("grid.png"));
  assert(grid.valid());

  if (CHECK_GL_ERRORS("Create textures"))
    return 1;

  // Texture array -------------------------------------------------------------

  // So that this matches what OpenGL expects.
  stbi_set_flip_vertically_on_load(true);
  int x, y, channels;

  uint8_t* data = stbi_load(Assets::TexturePath("grid.png").data(),
                            &x, &y, &channels, 4);
  (void)data;
  // Grid is square.
  assert(x == y);

  constexpr int side_count = 16;
  int elem_side = x / side_count;
  TextureArray2D tex_array({elem_side, elem_side, side_count * side_count},
                           side_count, GL_RGBA);
  (void)tex_array;
  /* if (!tex_array.Init()) */
  /*   return 1; */

  /* for (int i = 0; i < tex_array.size().z; i++) { */
  /*   int offset = elem_side * i; */
  /*   uint8_t elem_data[elem_side * elem_side]; */
  /*   for (int v = 0; v < elem_side; v++) { */
  /*     for (int u = 0; u < elem_side; u++) { */


  /*     } */
  /*   } */
  /*   if (!tex_array.AddElement(data + offset, */

  /* } */



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

  if (CHECK_GL_ERRORS("Creating voxel terrain"))
    return 1;

  // Camera --------------------------------------------------------------------

  float camera_speed = 5.0f;
  Camera camera(&sdl_context, {-10.0f, 5.0f, 10.0f});
  camera.SetTarget({});
  camera.UpdateView();

  // Game loop -----------------------------------------------------------------

  InputState input = InputState::Create();

  struct UIState {
    bool polygon_mode = false;
  };
  UIState ui_state = {};

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
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    } else {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    // Draw the triangle.
    glClearColor(0.137f, 0.152f, 0.637f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (CHECK_GL_ERRORS("Clear"))
      return 1;

    shader.Use();
    camera.SetProjection(&shader);
    camera.SetView(&shader);

    if (CHECK_GL_ERRORS("Setting Camera"))
      return 1;

    // Draw the cubes.
    glBindVertexArray(cube_vao);
    // Set the cube textures.
		wall.Set(&shader, GL_TEXTURE0);
    face.Set(&shader, GL_TEXTURE1);
    shader.SetMat4("model", glm::translate(glm::mat4(1.0f), {5.0f, 0.0, 0.0f}));
    glDrawArrays(GL_TRIANGLES, 0, 36);


    if (CHECK_GL_ERRORS("Drawing cube"))
      return 1;

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

    for (size_t i = 0; i < ARRAY_SIZE(cube_positions); i++) {
      glm::mat4 model = glm::translate(glm::mat4(1.0f), cube_positions[i]);
      float angle = sdl_context.seconds() * glm::radians(20.0f * i);
      model =
          glm::rotate(model, angle, glm::vec3(1.0f, 0.3f, 0.5f));
      shader.SetMat4("model", model);

      /* glDrawArrays(GL_TRIANGLES, 0, 36); */
    }

    if (CHECK_GL_ERRORS("Drawing minecraft cube"))
      return 1;

    // We only need one texture for the plane.
    grid.Set(&shader, GL_TEXTURE0);

    // Draw the plane.
    alpha_test_shader.Use();
    camera.SetProjection(&alpha_test_shader);
    camera.SetView(&alpha_test_shader);

    glBindVertexArray(plane_vao);
    // The model at the origin.
    shader.SetMat4("model", glm::mat4(1.0f));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


    voxel_shader.Use();
    camera.SetProjection(&voxel_shader);
    camera.SetView(&voxel_shader);
    terrain.Render(&voxel_shader);


    if (CHECK_GL_ERRORS("Drawing plane"))
      return 1;

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
