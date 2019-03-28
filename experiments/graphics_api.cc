// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <thread>

#include <warhol/assets/asset_paths.h>
#include <warhol/graphics/common/mesh.h>
#include <warhol/graphics/common/shader.h>
#include <warhol/graphics/common/renderer.h>
#include <warhol/input/input.h>
#include <warhol/scene/camera.h>
#include <warhol/utils/log.h>
#include <warhol/window/common/window.h>

#include <warhol/memory/memory_pool.h>

#include <warhol/utils/glm_impl.h>

using namespace warhol;

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

  LOG(DEBUG) << "Initializing window.";

  Window window;
  if (!InitWindow(&window, WindowBackendType::kSDLOpenGL)) {
    LOG(ERROR) << "Could not start SDL.";
    return 1;
  }

  LOG(DEBUG) << "Initializing renderer.";

  Renderer renderer;
  if (!InitRenderer(&renderer, RendererType::kOpenGL, &window)) {
    LOG(ERROR) << "Could not start renderer.";
    return 1;
  }

  LOG(DEBUG) << "Loading shaders.";

  Shader shader;
  const char* shader_name = "common";
  if (!LoadShader(GetShaderPath(shader_name, ShaderPathType::kOpenGL),
                  &shader)) {
    LOG(ERROR) << "Could not load shader " << shader_name;
    return 1;
  }

  if (!RendererStageShader(&renderer, &shader)) {
    LOG(ERROR) << "Could not load shader " << shader_name;
    return 1;
  }

  LOG(DEBUG) << "Loading meshes.";

  Mesh mesh;
  mesh.uuid = GetNextMeshUUID();    // Created by hand.
  mesh.vertices = vertices;
  mesh.indices = indices;

  if (!RendererStageMesh(&renderer, &mesh)) {
    LOG(ERROR) << "Could not load mesh into renderer.";
    return 1;
  }

  LOG(DEBUG) << "Loading textures.";

  Texture texture;
  TextureType texture_type = TextureType::kOpenGL;
  const char* texture_name = "awesomeface.png";
  if (!LoadTexture(GetTexturePath(texture_name), texture_type, &texture)) {
    LOG(ERROR) << "Could not load texture " <<  texture_name;
    return 1;
  }

  if (!RendererStageTexture(&renderer, &texture)) {
    LOG(ERROR) << "Could not load texture " << texture_name
               << " into renderer.";
    return 1;
  }

  LOG(DEBUG) << "Setting Memory Pool.";

  // Start pushing rendering actions.
  MemoryPool memory_pool;
  InitMemoryPool(&memory_pool, MEGABYTES(1));


  LOG(DEBUG) << "Setting camera.";

  Camera camera;
  camera.view = glm::lookAt(glm::vec3{2.0f, 2.0f, 2.0f}, {}, {0, 0, 0.1f});
  camera.projection =
      glm::perspective(glm::radians(45.0f),
                       (float)window.width / (float)window.height,
                       0.1f, 100.f);

  LOG(DEBUG) << "Set rendering commands.";

  LinkedList<MeshRenderAction> mesh_action_list;
  auto* mesh_action = PushIntoListFromPool(&mesh_action_list, &memory_pool);
  mesh_action->mesh = &mesh;

  LinkedList<RenderCommand> command_list;
  auto* command = PushIntoListFromPool(&command_list, &memory_pool);

  command->type = RenderCommandType::kMesh;
  command->camera = &camera;
  command->shader = &shader;
  command->mesh_actions = &mesh_action_list;

  LOG(DEBUG) << "Staring game loop.";

  InputState input = InputState::Create();

  Vec3 delta;
  bool running = true;
  while (running) {
    auto events = UpdateWindow(&window, &input);
    for (WindowEvent event : events) {
      if (event == WindowEvent::kQuit) {
        running = false;
        break;
      }
    }

    if (!running || input.keys_up[GET_KEY(Escape)])
      break;

    delta.x += 0.1f * window.frame_delta;
    delta.y += 0.2f * window.frame_delta;
    delta.z += 0.05f * window.frame_delta;
    delta *= 1.1f;

    if (delta.x > 1.0f) delta.x -= 1.0f;
    if (delta.y > 1.0f) delta.y -= 1.0f;
    if (delta.z > 1.0f) delta.z -= 1.0f;


    renderer.clear_color = delta;

    RendererStartFrame(&renderer);
    RendererExecuteCommands(&renderer, &command_list);
    RendererEndFrame(&renderer);

    WindowSwapBuffers(&window);

    std::this_thread::sleep_for(std::chrono::milliseconds(33));
  }

  LOG(DEBUG) << "Adios, amigo!";
}
