// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "experiments/new_api/drawer.h"

#include <warhol/utils/assert.h>
#include <warhol/window/window.h>


using namespace warhol;


void InitDrawer(Drawer* drawer, Window* window, Renderer* renderer) {
  ASSERT(!Valid(drawer));
  ASSERT(Valid(window));

  InitMemoryPool(&drawer->pool, KILOBYTES(64));
  InitMeshPools(&drawer->mesh, MEGABYTES(1), MEGABYTES(1));
  drawer->mesh.vertex_size = sizeof(DrawVertex);
  RendererStageMesh(renderer, &drawer->mesh);

  drawer->window = window;
}

void DrawerStartFrame(Drawer* drawer) {
  ResetMesh(&drawer->mesh);
  drawer->square_count = 0;

}

static inline void SetColor(uint8_t* src, uint8_t* dst) {
  *src++ = *dst++;
  *src++ = *dst++;
  *src++ = *dst++;
  *src++ = *dst++;
}

void PushSquare(Drawer* drawer, Square square) {
  uint32_t v_base = drawer->mesh.vertex_count;

  DrawVertex v[4];
  v[0].pos[0] = square.bottom_left.x;
  v[0].pos[1] = square.bottom_left.y;
  SetColor(v[0].color, square.color);
  v[1].pos[0] = square.top_right.x;
  v[1].pos[1] = square.bottom_left.y;
  SetColor(v[1].color, square.color);
  v[2].pos[0] = square.top_right.x;
  v[2].pos[1] = square.top_right.y;
  SetColor(v[2].color, square.color);
  v[3].pos[0] = square.bottom_left.x;
  v[3].pos[1] = square.top_right.y;
  SetColor(v[3].color, square.color);
  PushVertices(&drawer->mesh, v, 4);

  uint32_t indices[6];
  indices[0] = v_base;
  indices[1] = v_base + 1;
  indices[2] = v_base + 2;
  indices[3] = v_base + 2;
  indices[4] = v_base + 3;
  indices[5] = v_base;
  PushIndices(&drawer->mesh, indices, 6);

  drawer->square_count++;
}


RenderCommand DrawerEndFrame(Drawer* drawer) {
  LinkedList<MeshRenderAction> actions;

  MeshRenderAction action;
  action.mesh = &drawer->mesh;
  action.index_range = CreateRange(drawer->square_count * 2, 0);

  PushIntoListFromMemoryPool(&actions, &drawer->pool, std::move(action));

  RenderCommand command;
  command.name = "Squares";
  command.type = RenderCommandType::kMesh;
  command.camera = &drawer->camera;
  command.shader = &drawer->shader;
  command.actions.mesh_actions = std::move(actions);

  return command;
}
