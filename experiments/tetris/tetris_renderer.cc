// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "tetris_renderer.h"

#include <warhol/utils/glm_impl.h>

#include <map>

#include "game.h"
#include "tetris.h"

namespace tetris {

TetrisRenderer::~TetrisRenderer() {
  if (Valid(this))
    Shutdown(this);
}

bool Valid(TetrisRenderer* renderer) {
  return renderer->shader.uuid.has_value();
}

// Init ------------------------------------------------------------------------

namespace {

struct TetrisRendererVertex {
  Vec2 pos;
};

}  // namespace

bool Init(TetrisRenderer* tetris_renderer, Renderer* renderer, Window* window) {
  NOT_IMPLEMENTED();
  ASSERT(!Valid(tetris_renderer));

  /* if (!LoadShader("direct", "skydome", &tetris_renderer->shader)) { */
  /*   LOG(ERROR) << "Could not load shader!"; */
  /*   return false; */
  /* } */

  tetris_renderer->mesh.name = "TetrisRenderer";

  // Create a mesh for creating a buffer.
  tetris_renderer->mesh.name = "TetrisRendererMesh";
  tetris_renderer->mesh.uuid = GetNextMeshUUID();
  tetris_renderer->mesh.vertex_size = sizeof(TetrisRendererVertex);
  tetris_renderer->mesh.attributes = {
    {2, AttributeType::kFloat, false},
  };

  InitMeshPools(&tetris_renderer->mesh, KILOBYTES(16), KILOBYTES(16));
  if (!RendererStageMesh(renderer, &tetris_renderer->mesh)) {
    LOG(ERROR) << "Could not stage mesh!";
    return false;
  }

  tetris_renderer->renderer = renderer;
  tetris_renderer->window = window;

  InitMemoryPool(&tetris_renderer->pool, KILOBYTES(2));

  tetris_renderer->camera.projection = glm::mat4(1.0f);
  tetris_renderer->camera.view = glm::mat4(1.0f);

  return true;
}

// Shutdown --------------------------------------------------------------------

void Shutdown(TetrisRenderer* renderer) {
  ASSERT(Valid(renderer));
  RendererUnstageMesh(renderer->renderer, &renderer->mesh);
  renderer->mesh = {};
  RendererUnstageShader(renderer->renderer, &renderer->shader);
  renderer->shader = {};
}

// New Frame -------------------------------------------------------------------

void NewFrame(TetrisRenderer* tetris_renderer) {
  ResetMemoryPool(&tetris_renderer->pool);
  ResetMesh(&tetris_renderer->mesh);
}

// EndFrame --------------------------------------------------------------------

namespace {

void AddQuad(Mesh* mesh, Window* window) {
  TetrisRendererVertex vertices[4];
  vertices[0].pos = { 0.0f, 0.0f };
  vertices[1].pos = { (float)window->width, 0.0f};
  vertices[2].pos = { 0.0f, (float)window->height };
  vertices[3].pos = { (float)window->width, (float)window->height };

  uint32_t indices[6];
  indices[0] = 0;
  indices[1] = 1;
  indices[2] = 2;
  indices[3] = 2;
  indices[4] = 3;
  indices[5] = 0;

  PushVertices(mesh, vertices, ARRAY_SIZE(vertices));
  PushIndices(mesh, indices, ARRAY_SIZE(indices));
}

void AddUniforms(MeshRenderAction* action, Window* window, MemoryPool* pool) {
  uint8_t* ptr = pool->current;
  Push(pool, &window->width, 1);
  Push(pool, &window->height, 1);

  float f;
  f = 49.0f / 255.0f; Push(pool, &f, 1);
  f = 33.0f / 255.0f; Push(pool, &f, 1);
  f = 66.0f / 255.0f; Push(pool, &f, 1);

  f = 0.0f; Push(pool, &f, 1);
  f = 0.05f; Push(pool, &f, 1);
  f = 0.2f; Push(pool, &f, 1);

  action->frag_uniforms = ptr;
}

}  // namespace

RenderCommand EndFrame(TetrisRenderer* renderer) {
  SCOPE_LOCATION();

  renderer->camera.viewport_p1 = {0, 0};
  renderer->camera.viewport_p2 = {renderer->window->width,
                                  renderer->window->height};

  // TODO(Cristian): Move this to camera.
  float L = renderer->camera.viewport_p1.x;
  float R = renderer->camera.viewport_p2.x;
  float T = renderer->camera.viewport_p1.y;
  float B = renderer->camera.viewport_p2.y;
  renderer->camera.projection = glm::ortho(L, R, B, T);

  // Send the frame over.
  AddQuad(&renderer->mesh, renderer->window);
  if (RendererUploadMeshRange(renderer->renderer, &renderer->mesh))
    NOT_REACHED() << "Could not upload mesh range";

  MeshRenderAction action;
  action.mesh = &renderer->mesh;
  action.index_range = CreateRange(renderer->mesh.index_count, 0);
  AddUniforms(&action, renderer->window, &renderer->pool);

  auto actions = CreateList<MeshRenderAction>(&renderer->pool);
  Push(&actions, std::move(action));

  RenderCommand render_command;
  render_command.name = "TetrisRendererSkybox";
  render_command.type = RenderCommandType::kMesh;
  render_command.config.blend_enabled = false;
  render_command.config.cull_faces = false;
  render_command.config.depth_test = false;
  render_command.config.scissor_test = false;
  render_command.config.wireframe_mode = false;
  render_command.camera = &renderer->camera;
  render_command.shader = &renderer->shader;
  render_command.actions.mesh_actions = std::move(actions);

  return render_command;
}

// Drawing ---------------------------------------------------------------------

namespace {

void DrawBackground(Game* game, Tetris* tetris) {
  // Generate the board.
  Board* board = &tetris->board;

  auto& dims = tetris->dimensions;
  int right_pad = dims.screen_pad + dims.board_width;

  int border = 3;
  // Draw border.
  DrawSquare(&tetris->drawer,
             {dims.screen_pad - border, 0},
             {dims.screen_pad, game->window.height},
             Colors::kTeal);

  DrawSquare(&tetris->drawer,
             {right_pad, 0},
             {right_pad + border, game->window.height},
             Colors::kTeal);

  // Draw the grid.
  for (int y = 1; y < (int)board->height; y++) {
    DrawSquare(&tetris->drawer,
               {dims.screen_pad, y * dims.block_size},
               {right_pad, y * dims.block_size - 1},
               Colors::kGray);
  }

  for (int x = 1; x < (int)board->width; x++) {
    DrawSquare(&tetris->drawer,
               {dims.screen_pad + x * dims.block_size, 0},
               {dims.screen_pad + x * dims.block_size - 1, game->window.height},
               Colors::kGray);
  }
}

int BlockTypeToColor(uint8_t type) {
  static std::map<uint8_t, int> kColorMap = {
    {kLiveBlock, Colors::kBlue},
    {kDeadBlock, Colors::kRed},
    {kPivot, Colors::kGreen},
    {kShadow, Colors::kGray},
  };

  auto it = kColorMap.find(type);
  ASSERT(it != kColorMap.end());
  return it->second;
}

void DrawColoredSquare(Game* game, Tetris* tetris, uint8_t square, int x, int y) {
  auto& dims = tetris->dimensions;
  int ax = dims.screen_pad + x * dims.block_size;
  int ay = game->window.height - y * dims.block_size - dims.block_size;
  DrawSquare(&tetris->drawer,
             {ax, ay}, {ax + dims.block_size - 1, ay + dims.block_size - 1},
             BlockTypeToColor(square));
}

void DrawShadow(Game* game, Tetris* tetris, int x, int y) {
  auto& dims = tetris->dimensions;
  int ax = dims.screen_pad + x * dims.block_size;
  int ay = game->window.height - y * dims.block_size - dims.block_size;
  DrawBorderSquare(&tetris->drawer,
                   {ax, ay},
                   {ax + dims.block_size - 1, ay + dims.block_size - 1},
                   Colors::kTeal);
}

void DrawBlock(Game* game, Tetris* tetris, int x, int y) {
  uint8_t square = GetSquare(&tetris->board, x, y);
  switch (square) {
    case kLiveBlock:
    case kDeadBlock:
    case kPivot:
      return DrawColoredSquare(game, tetris, square, x, y);
    case kShadow:
      return DrawShadow(game, tetris, x, y);
    case kNone:
      return;
    default:
      break;
  }
}

void DrawBoard(Game* game, Tetris* tetris) {
  Board* board = &tetris->board;
  for (int y = 0; y < (int)board->height; y++) {
    for (int x = 0; x < (int)board->width; x++) {
      DrawBlock(game, tetris, x, y);
    }
  }
}

/* static inline int CreateColor(uint8_t i) { */
/*   return 0xff000000 | i << 16 | i << 8 | i; */
/* } */

/* void DrawDebugSquares(Game* game, Tetris* tetris) { */
/*   Board* board = &tetris->board; */
/*   for (int i = 0; i < board->height; i++) { */
/*     DrawBlock(game, tetris, CreateColor(i * 10), 0, i); */
/*   } */
/* } */

}  // namespace

RenderCommand GetTetrisRenderCommand(Game* game, Tetris* tetris) {
  tetris->dimensions = GetTetrisScreenDimensions(game, tetris);

  DrawBackground(game, tetris);
  DrawBoard(game, tetris);

  /* DrawDebugSquares(game, tetris); */

  return DrawerEndFrame(&tetris->drawer);
}

// Screen Dimensions -----------------------------------------------------------

TetrisScreenDimensions
GetTetrisScreenDimensions(Game* game, Tetris* tetris) {
  TetrisScreenDimensions dims;
  dims.block_size = game->window.height / tetris->board.height;
  dims.board_width = dims.block_size * tetris->board.width;
  dims.screen_pad = (game->window.width - dims.board_width) / 2;
  return dims;
}

}  // namespace tetris
