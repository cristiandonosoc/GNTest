// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <thread>

#include <warhol/assets/asset_paths.h>
#include <warhol/graphics/graphics.h>
#include <warhol/input/input.h>
#include <warhol/memory/memory_pool.h>
#include <warhol/memory/memory_tracker.h>
#include <warhol/platform/timing.h>
#include <warhol/scene/camera.h>
#include <warhol/ui/imgui.h>
#include <warhol/utils/glm_impl.h>
#include <warhol/utils/log.h>
#include <warhol/window/window.h>

#include "game.h"

using namespace warhol;
using namespace warhol::imgui;

namespace {

const std::vector<Vertex> vertices = {
  {{-0.5f, -0.5f,  0.0f},
    {1.0f,  0.0f,  0.0f},
   {0.0f,  0.0f}},

  {{0.5f, -0.5f,  0.0f},
    {0.0f,  1.0f,  0.0f},
    {1.0f,  0.0f}},

  {{0.5f,  0.5f,  0.0f},
    {0.0f,  0.0f,  1.0f},
    {1.0f,  1.0f}},

  {{-0.5f,  0.5f,  0.0f},
    {1.0f,  1.0f,  1.0f},
    {0.0f,  1.0f}},

  {{-0.5f, -0.5f,  -0.5f},
    {1.0f,  0.0f,  0.0f},
    {0.0f,  0.0f}},

  {{0.5f, -0.5f,  -0.5f},
    {0.0f,  1.0f,  0.0f},
    {1.0f,  0.0f}},

  {{0.5f,  0.5f,  -0.5f},
    {0.0f,  0.0f,  1.0f},
    {1.0f,  1.0f}},

  {{-0.5f,  0.5f,  -0.5f},
    {1.0f,  1.0f,  1.0f},
    {0.0f,  1.0f}},
};


}  // namespace

const std::vector<uint32_t> indices = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4,
};

int main() {
  Game game = {};
  if (!InitGame(&game, WindowBackendType::kSDLOpenGL, RendererType::kOpenGL)) {
    LOG(ERROR) << "Could not initialize game.";
    return 1;
  }

  LOG(DEBUG) << "Loading shaders.";

  Shader shader;
  const char* shader_name = "simple";
  if (!LoadShader("simple", &game.renderer, &shader)) {
    LOG(ERROR) << "Could not load shader " << shader_name;
    return 1;
  }

  shader.vert_ubo_size = sizeof(glm::mat4);
  shader.texture_count = 1;

  LOG(DEBUG) << "Is shader staged? "
             << RendererIsShaderStaged(&game.renderer, &shader);

  LOG(DEBUG) << "Loading meshes.";

  Mesh mesh;
  mesh.name = "SquareMesh";
  mesh.uuid = GetNextMeshUUID();    // Created by hand.
  mesh.attributes = {
    {3, AttributeType::kFloat},
    {3, AttributeType::kFloat},
    {2, AttributeType::kFloat},
  };

  mesh.vertex_size = sizeof(decltype(vertices)::value_type);
  mesh.vertex_count = vertices.size();
  mesh.index_count = indices.size();
  InitMeshPools(&mesh, KILOBYTES(1), KILOBYTES(1));
  /* // Making it precise to size. */
  /* InitMeshPools(&mesh, */
  /*               mesh.vertex_size * mesh.vertex_count, */
  /*               sizeof(uint32_t) * mesh.index_count); */
  Push(&mesh.vertices, vertices.data(), vertices.size());
  Push(&mesh.indices, indices.data(), indices.size());

  Track(&game.memory_tracker, &mesh);

  if (!RendererStageMesh(&game.renderer, &mesh)) {
    LOG(ERROR) << "Could not load mesh into renderer.";
    return 1;
  }

  LOG(DEBUG) << "Loading textures.";

  Texture texture;
  TextureType texture_type = TextureType::kOpenGL;
  const char* texture_name = "wall.jpg";
  if (!LoadTexture(GetTexturePath(texture_name), texture_type, &texture)) {
    LOG(ERROR) << "Could not load texture " <<  texture_name;
    return 1;
  }

  StageTextureConfig config = {};
  if (!RendererStageTexture(&game.renderer, &texture, &config)) {
    LOG(ERROR) << "Could not load texture " << texture_name
               << " into renderer.";
    return 1;
  }

  LOG(DEBUG) << "Setting Memory Pool.";

  // Start pushing rendering actions.
  MemoryPool memory_pool;
  memory_pool.name = "Main";
  InitMemoryPool(&memory_pool, MEGABYTES(1));

  Track(&game.memory_tracker, &memory_pool);

  LOG(DEBUG) << "Setting camera.";

  Camera camera;
  camera.view = glm::lookAt(glm::vec3{2.0f, 2.0f, 2.0f}, {}, {0, 0, 0.1f});
  camera.projection =
      glm::perspective(glm::radians(45.0f),
                       (float)game.window.width / (float)game.window.height,
                       0.1f, 100.f);

  LOG(DEBUG) << "Setting drawer.";

  Drawer drawer;
  InitDrawer(&drawer, &window);

  LOG(DEBUG) << "Staring game loop.";

  Vec3 delta;
  bool running = true;
  while (running) {
    auto events = NewFrame(&game);
    for (WindowEvent event : events) {
      if (event == WindowEvent::kQuit) {
        running = false;
        break;
      }
    }

    if (!running || game.input.keys_up[GET_KEY(Escape)])
      break;

    delta.x += 0.1f * game.time.frame_delta;
    delta.y += 0.2f * game.time.frame_delta;
    delta.z += 0.05f * game.time.frame_delta;
    /* game.renderer.clear_color = delta; */

    ResetMemoryPool(&memory_pool);

    auto mesh_action_list = CreateList<MeshRenderAction>(&memory_pool);
    auto* mesh_action = Push(&mesh_action_list);
    mesh_action->mesh = &mesh;
    mesh_action->index_range = CreateRange(mesh.index_count, 0);
    mesh_action->textures = &texture;

    auto* model = Push<glm::mat4>(&memory_pool);
    *model = glm::mat4(1);

    mesh_action->vert_uniforms = (float*)model;

    auto command_list = CreateList<RenderCommand>(&memory_pool);
    auto* command = Push(&command_list);

    command->name = "Loop command";
    command->type = RenderCommandType::kMesh;
    command->camera = &camera;
    command->shader = &shader;
    command->actions.mesh_actions = std::move(mesh_action_list);

    *model = glm::rotate(glm::mat4(1.0f),
                         game.time.seconds * glm::radians(90.0f),
                         glm::vec3(0, 0, 1));


    int sq = 10;  // square width.
    int b = 2;    // border.
    int y = 0;
    while (y < game.window.height) {
      int x = 0;

      while (x < game.window.width) {

#ifdef AAAA
        float u = 0.5f - ((float)x / (float)game.window.width);
        float v = 0.5f - ((float)y / (float)game.window.height);

        float inar = (float)game.window.height / (float)game.window.width;
        v *= inar;

        Vec3 col;
        col.x = u;
        col.y = v;
        col.z = 0.5f + 0.5f * sin(game.time.seconds);


        float r = -sqrt(u*u + v*v);
        float z = 1.0f + 0.5f * sin(r + game.time.seconds * 0.35f) / 0.013f;

        col.x *= z;
        col.y *= z;
        col.z *= z;


#endif

        float u = 0.5f - ((float)x / (float)game.window.width);
        float v = 0.5f - ((float)y / (float)game.window.height);

        Vec3 color = {};

        float r = 0.5f + 0.5f* sin(sqrt(u*u + v*v) * 25 * game.time.seconds);
        color.x = r * sin(delta.x);
        color.y = r * sin(delta.y);
        color.z = r * sin(delta.z);

        DrawSquare(&game.drawer, {x + b, y + b}, {x + sq - b, y + sq - b},
                   color);


        x += sq;
      }
      y += sq;
    }

    Push(&command_list, ImguiEndFrame(&game.imgui));
    Push(&command_list, DrawerEndFrame(&game.drawer));

    EndFrame(&game, std::move(command_list));
  }

  RendererUnstageMesh(&game.renderer, &mesh);

  LOG(DEBUG) << "Adios, amigo!";
}
