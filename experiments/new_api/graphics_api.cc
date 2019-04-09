// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <thread>

#include <warhol/assets/asset_paths.h>
#include <warhol/graphics/common/mesh.h>
#include <warhol/graphics/common/shader.h>
#include <warhol/graphics/common/renderer.h>
#include <warhol/input/input.h>
#include <warhol/platform/timing.h>
#include <warhol/scene/camera.h>
#include <warhol/ui/imgui.h>
#include <warhol/utils/log.h>
#include <warhol/window/common/window.h>


#include <warhol/memory/memory_pool.h>

#include <warhol/utils/glm_impl.h>

using namespace warhol;
using namespace warhol::imgui;

namespace {

std::vector<MemoryPool*> tracked_pools;
std::vector<Mesh*> tracked_meshes;

void TrackMemoryPool(MemoryPool* pool) {
  ASSERT(!Active(&pool->track_guard));

  tracked_pools.push_back(pool);
  pool->track_guard.active = true;
}

void TrackMesh(Mesh* mesh) {
  ASSERT(!Active(&mesh->track_guard));

  tracked_meshes.push_back(mesh);
  mesh->track_guard.active = true;
}

void CreateImguiUI(PlatformTime* time) {
  ImGui::Begin("NEW API");
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
              1000.0f * time->frame_delta_average,
              time->frame_rate);
  ImGui::LabelText("Seconds", "%.3f", time->seconds);

  ImGui::Separator();

  for (MemoryPool* pool : tracked_pools) {
    float used_ratio = (float)Used(pool) / (float)pool->size;
    auto used_str = BytesToString(Used(pool));
    auto total_str = BytesToString(pool->size);
    auto bar = StringPrintf("%s/%s", used_str.c_str(), total_str.c_str());

    ImGui::ProgressBar(used_ratio, {0, 0}, bar.c_str());
    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::Text("%s", pool->name);
  }

  for (Mesh* mesh : tracked_meshes) {
    {
      MemoryPool* pool = &mesh->vertices;
      float used_ratio = (float)Used(pool) / (float)pool->size;
      auto used_str = BytesToString(Used(pool));
      auto total_str = BytesToString(pool->size);
      auto bar = StringPrintf("%s/%s", used_str.c_str(), total_str.c_str());

      ImGui::ProgressBar(used_ratio, {0, 0}, bar.c_str());

      auto pool_name = StringPrintf("%s.%s", mesh->name, pool->name);
      ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
      ImGui::Text("%s", pool_name.c_str());
    }

    {
      MemoryPool* pool = &mesh->indices;
      float used_ratio = (float)Used(pool) / (float)pool->size;
      auto used_str = BytesToString(Used(pool));
      auto total_str = BytesToString(pool->size);
      auto bar = StringPrintf("%s/%s", used_str.c_str(), total_str.c_str());

      ImGui::ProgressBar(used_ratio, {0, 0}, bar.c_str());

      auto pool_name = StringPrintf("%s.%s", mesh->name, pool->name);
      ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
      ImGui::Text("%s", pool_name.c_str());
    }

  }

  ImGui::End();
}

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
  PlatformTime time;


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

  LOG(DEBUG) << "Initializing imgui.";

  ImguiContext imgui_context;
  if (!InitImgui(&renderer, &imgui_context)) {
    LOG(ERROR) << "Could not start imgui.";
    return 1;
  }

  LOG(DEBUG) << "Loading shaders.";

  Shader shader;
  const char* shader_name = "simple";
  if (!LoadShader("simple_shader",
                  GetShaderPath(shader_name, ShaderPathType::kOpenGL),
                  ShaderType::kOpenGL,
                  &shader)) {
    LOG(ERROR) << "Could not load shader " << shader_name;
    return 1;
  }

  shader.vert_ubo_size = sizeof(glm::mat4);
  shader.texture_count = 1;

  if (!RendererStageShader(&renderer, &shader)) {
    LOG(ERROR) << "Could not load shader " << shader_name;
    return 1;
  }

  LOG(DEBUG) << "Is shader staged? "
             << RendererIsShaderStaged(&renderer, &shader);

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
  PushIntoMemoryPool(&mesh.vertices, vertices.data(), vertices.size());
  PushIntoMemoryPool(&mesh.indices, indices.data(), indices.size());

  TrackMesh(&mesh);

  if (!RendererStageMesh(&renderer, &mesh)) {
    LOG(ERROR) << "Could not load mesh into renderer.";
    return 1;
  }

  LOG(DEBUG) << "Is mesh staged? " << RendererIsMeshStaged(&renderer, &mesh);

  LOG(DEBUG) << "Loading textures.";

  Texture texture;
  TextureType texture_type = TextureType::kOpenGL;
  const char* texture_name = "wall.jpg";
  if (!LoadTexture(GetTexturePath(texture_name), texture_type, &texture)) {
    LOG(ERROR) << "Could not load texture " <<  texture_name;
    return 1;
  }

  if (!RendererStageTexture(&renderer, &texture)) {
    LOG(ERROR) << "Could not load texture " << texture_name
               << " into renderer.";
    return 1;
  }

  LOG(DEBUG) << "Is texture staged? "
             << RendererIsTextureStaged(&renderer, &texture);

  LOG(DEBUG) << "Setting Memory Pool.";

  // Start pushing rendering actions.
  MemoryPool memory_pool;
  memory_pool.name = "Main";
  InitMemoryPool(&memory_pool, MEGABYTES(1));

  TrackMemoryPool(&memory_pool);

  LOG(DEBUG) << "Setting camera.";

  Camera camera;
  camera.view = glm::lookAt(glm::vec3{2.0f, 2.0f, 2.0f}, {}, {0, 0, 0.1f});
  camera.projection =
      glm::perspective(glm::radians(45.0f),
                       (float)window.width / (float)window.height,
                       0.1f, 100.f);

  LOG(DEBUG) << "Staring game loop.";

  InputState input = InputState::Create();

  Vec3 delta;
  bool running = true;
  while (running) {
    PlatformUpdateTiming(&time);

    auto events = UpdateWindow(&window, &input);
    for (WindowEvent event : events) {
      if (event == WindowEvent::kQuit) {
        running = false;
        break;
      }
    }

    ImguiStartFrame(&window, &time, &input, &imgui_context);
    /* if (imgui_context.keyboard_captured || imgui_context.mouse_captured) */
    /*   LOG(DEBUG) << "Captured."; */

    if (!running || input.keys_up[GET_KEY(Escape)])
      break;

    delta.x += 0.1f * time.frame_delta;
    delta.y += 0.2f * time.frame_delta;
    delta.z += 0.05f * time.frame_delta;
    renderer.clear_color = delta;

    ResetMemoryPool(&memory_pool);

    LinkedList<MeshRenderAction> mesh_action_list;
    auto* mesh_action = PushIntoListFromMemoryPool(&mesh_action_list,
                                                   &memory_pool);
    mesh_action->mesh = &mesh;
    mesh_action->index_range = CreateRange(mesh.index_count, 0);
    mesh_action->textures = &texture;

    auto* model = PushIntoMemoryPool<glm::mat4>(&memory_pool);
    *model = glm::mat4(1);

    mesh_action->vert_uniforms = (float*)model;

    LinkedList<RenderCommand> command_list;
    auto* command = PushIntoListFromMemoryPool(&command_list, &memory_pool);

    command->name = "Loop command";
    command->type = RenderCommandType::kMesh;
    command->camera = &camera;
    command->shader = &shader;
    command->actions.mesh_actions = std::move(mesh_action_list);

    *model = glm::rotate(glm::mat4(1.0f),
                         time.seconds * glm::radians(90.0f),
                         glm::vec3(0, 0, 1));

    ImGui::ShowDemoWindow();
    CreateImguiUI(&time);

    RenderCommand imgui_command = ImguiEndFrame(&imgui_context);
    PushIntoListFromMemoryPool(&command_list, &memory_pool,
                               std::move(imgui_command));

    RendererStartFrame(&renderer);
    RendererExecuteCommands(&renderer, &command_list);
    RendererEndFrame(&renderer);

    WindowSwapBuffers(&window);
  }

  RendererUnstageMesh(&renderer, &mesh);

  LOG(DEBUG) << "Adios, amigo!";
}
