// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "drawer.h"

#include <warhol/assets/asset_paths.h>
#include <warhol/utils/glm_impl.h>
#include <warhol/window/window.h>

struct DrawerVertex {
  Vec2 pos;
  uint32_t color;
};

bool InitDrawer(Drawer* drawer, Renderer* renderer, Window* window) {
  if (!LoadShader("square", renderer, &drawer->shader)) {
    LOG(ERROR) << "Could not load shader!";
    return false;
  }

  drawer->mesh.name = "DrawerMesh";

  // Create a Mesh for creating a buffer.
  drawer->mesh.name = "Imgui Mesh";
  drawer->mesh.uuid = GetNextMeshUUID();
  drawer->mesh.vertex_size = sizeof(DrawerVertex);
  drawer->mesh.attributes = {
    {2, AttributeType::kFloat, false},  // Pos.
    {4, AttributeType::kUint8, true},   // Color.
  };

  InitMeshPools(&drawer->mesh, MEGABYTES(16), MEGABYTES(16));

  if (!RendererStageMesh(renderer, &drawer->mesh))
    return false;

  drawer->renderer = renderer;
  drawer->window = window;

  InitMemoryPool(&drawer->pool, KILOBYTES(1));

  drawer->camera.projection = glm::mat4(1.0f);
  drawer->camera.view = glm::mat4(1.0f);

  return true;
}

Drawer::~Drawer() {
  if (Valid(this))
    ShutdownDrawer(this);
}

void ShutdownDrawer(Drawer* drawer) {
  ASSERT(Valid(drawer));
  RendererUnstageMesh(drawer->renderer, &drawer->mesh);
  drawer->mesh = {};
  RendererUnstageShader(drawer->renderer, &drawer->shader);
  drawer->shader = {};
}

bool Valid(Drawer* drawer) {
  return drawer->renderer && drawer->window;
}

void DrawerNewFrame(Drawer* drawer) {
  ResetMemoryPool(&drawer->pool);
  ResetMesh(&drawer->mesh);
}

void DrawSquare(Drawer* drawer, Pair<int> bl, Pair<int> tr, uint32_t color) {
  SCOPE_LOCATION();

  DrawerVertex vertices[4];
  vertices[0].pos = {(float)bl.x, (float)bl.y};
  vertices[0].color = color;

  vertices[1].pos = {(float)tr.x, (float)bl.y};
  vertices[1].color = color;

  vertices[2].pos = {(float)tr.x, (float)tr.y};
  vertices[2].color = color;

  vertices[3].pos = {(float)bl.x, (float)tr.y};
  vertices[3].color = color;

  uint32_t vert_count = drawer->mesh.vertex_count;
  uint32_t indices[6];
  indices[0] = vert_count + 0;
  indices[1] = vert_count + 1;
  indices[2] = vert_count + 2;
  indices[3] = vert_count + 2;
  indices[4] = vert_count + 3;
  indices[5] = vert_count + 0;

  PushVertices(&drawer->mesh, vertices, ARRAY_SIZE(vertices));
  PushIndices(&drawer->mesh, indices, 6);
};

RenderCommand DrawerEndFrame(Drawer* drawer) {
  SCOPE_LOCATION();

  drawer->camera.viewport_p1 = {0, 0};
  drawer->camera.viewport_p2 = {drawer->window->width, drawer->window->height};

  float L = drawer->camera.viewport_p1.x;
  float R = drawer->camera.viewport_p2.x;
  float T = drawer->camera.viewport_p1.y;
  float B = drawer->camera.viewport_p2.y;
  drawer->camera.projection = glm::ortho(L, R, B, T);

  // Send the frame over.
  if (!RendererUploadMeshRange(drawer->renderer, &drawer->mesh))
    NOT_REACHED("ERROR");

  MeshRenderAction action;
  action.mesh = &drawer->mesh;
  action.index_range = CreateRange(drawer->mesh.index_count, 0);

  auto actions = CreateList<MeshRenderAction>(&drawer->pool);
  Push(&actions, std::move(action));

  RenderCommand render_command;
  render_command.name = "Imgui";
  render_command.type = RenderCommandType::kMesh;
  render_command.config.blend_enabled = false;
  render_command.config.cull_faces = false;
  render_command.config.depth_test = false;
  render_command.config.scissor_test = false;
  render_command.config.wireframe_mode = true;
  render_command.camera = &drawer->camera;
  render_command.shader = &drawer->shader;
  render_command.actions.mesh_actions = std::move(actions);

  return render_command;
}
