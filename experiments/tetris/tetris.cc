// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "tetris.h"

#include <stdlib.h>

#include <map>

#include "game.h"

namespace tetris {

bool InitTetris(Game* game, Tetris* tetris) {
  Board board = {};
  board.width = 10;
  board.height = 20;
  uint32_t slot_count = board.width * board.height;
  board.slots.resize(slot_count);
  for (size_t i = 0; i < slot_count; i++) {
    board.slots[i] = 0;
  }

  *tetris = {};
  tetris->board = std::move(board);

  return InitDrawer(&tetris->drawer, &game->renderer, &game->window);
}

// Update ----------------------------------------------------------------------

namespace {


const Shape kShapes[] = {
    {{{0, 0}, {-1, 0}, {-1, 1}, {0, 1}}},
};

enum class ShapeIndex {
  kSquare = 0,
  kLast,
};
static_assert(ARRAY_SIZE(kShapes) - 1 < (size_t)ShapeIndex::kLast);

#define GET_SHAPE_PTR(shape_name) \
  (&kShapes[(uint32_t)ShapeIndex::k##shape_name])
#define GET_SHAPE(shape_name) (*GET_SHAPE_PTR(shape_name))

};  // namespace

namespace {

bool HasCurrentShape(Tetris* tetris) {
  return Valid(&tetris->current_shape.shape);
}

void ClearLiveShape(Tetris* tetris) {
  // We clear the live blocks.
  Board* board = &tetris->board;
  for (int y = 0; y < board->height; y++) {
    for (int x = 0; x < board->width; x++) {
      int index = CoordToIndex(board, x, y);
      if (index < 0)
        continue;
      if (board->slots[index] == kLiveBlock)
        board->slots[index] = 0;
    }
  }
}

void PlaceCurrentShape(Tetris* tetris) {
  if (!HasCurrentShape(tetris))
    return;

  Int2 pos = tetris->current_shape.pos;
  Board* board = &tetris->board;
  board->slots[CoordToIndex(board, tetris->current_shape.pos)] = kLiveBlock;
  for (auto& offset : tetris->current_shape.shape.offsets) {
    auto new_pos = pos + offset;
    if (WithinBounds(board, new_pos))
      board->slots[CoordToIndex(board, new_pos)] = kLiveBlock;
  }
}

}  // namespace

// Update ----------------------------------------------------------------------

namespace {

void UpdateBoard(Tetris* tetris) {
  ClearLiveShape(tetris);
  PlaceCurrentShape(tetris);
}

void UpdateNewShape(Game* game, Tetris* tetris) {
  if (tetris->last_shape_time + tetris->next_shape_tick > game->time.seconds)
    return;
  tetris->last_shape_time = game->time.seconds;

  // Create a new shape.
  Board* board = &tetris->board;
  tetris->current_shape.shape = GET_SHAPE(Square);
  Int2 new_pos;
  new_pos.y = board->height - 1;
  new_pos.x = rand() % board->width;
  tetris->current_shape.pos = new_pos;
  LOG(DEBUG) << "Creating new shape at: " << ToString(new_pos);

}

// Returns an offset of where the shape should move.
Int2 UpdateSideMove(Game* game, Tetris* tetris) {
  if (tetris->last_side_move + tetris->side_move_tick > game->time.seconds)
    return {};
  tetris->last_side_move = game->time.seconds;

  // Left trumps right (down with capitalism!).
  if (game->input.left)
    return {-1, 0};
  else
    return {1, 0};


}

Int2 UpdateDownMovement(Game* game, Tetris* tetris) {
  // Move the shape down anyway.
  if (tetris->last_move_time + tetris->move_tick > game->time.seconds)
    return {};

  tetris->last_move_time = game->time.seconds;
  return {0, -1};
}

void DoShapeCollision(Tetris* tetris, Int2 offset) {
  // We cristalize the shape.
  for (Int2& sqr_offset : tetris->current_shape.shape.offsets) {
    Int2 sqr_pos = tetris->current_shape.pos + sqr_offset + offset;
    tetris->board.slots[CoordToIndex(&tetris->board, sqr_pos)] = kDeadBlock;
  }

  tetris->current_shape = {};
}

void UpdateCurrentShape(Game* game, Tetris* tetris) {
  SCOPE_LOCATION();
  Int2 side_offset = {};
  if (game->input.left || game->input.right)
    side_offset = UpdateSideMove(game, tetris);
  Int2 down_offset = UpdateDownMovement(game, tetris);

  Int2 offset = side_offset + down_offset;
  if (offset == Int2::Zero())
    return;

  LOG(DEBUG) << "Pos: " << ToString(tetris->current_shape.pos)
             << ", Offset: " << ToString(offset);

  auto collision_type = CheckShapeCollision(&tetris->board,
                                            &tetris->current_shape.shape,
                                            tetris->current_shape.pos,
                                            offset);

  LOG(DEBUG) << "Collision Type: " << CollisionTypeToString(collision_type);

  switch (collision_type) {
    case CollisionType::kNone:
      LOG(DEBUG) << "Before: " << ToString(tetris->current_shape.pos);
      tetris->current_shape.pos += offset;
      LOG(DEBUG) << "After: " << ToString(tetris->current_shape.pos);
      return;
    case CollisionType::kBorder:
      // Ignore side offset.
      /* tetris->current_shape.pos += down_offset; */
      return;
    case CollisionType::kBottom:
      DoShapeCollision(tetris, down_offset);
      return;
    case CollisionType::kShape:
      /* DoShapeCollision(tetris, offset); */
      return;
  }
}

}  // namespace

void UpdateTetris(Game* game, Tetris* tetris) {
  // We see if we need to createa a new shape.
  if (!HasCurrentShape(tetris)) {
    UpdateNewShape(game, tetris);
  } else {
    UpdateCurrentShape(game, tetris);
  }

  UpdateBoard(tetris);

  DrawerNewFrame(&tetris->drawer);
}

// End Frame -------------------------------------------------------------------

namespace {

void DrawBackground(Game* game, Tetris* tetris) {
  // Generate the board.
  Board* board = &tetris->board;
  int side = game->window.height / 20;
  int width = side * tetris->board.width;
  int left = (game->window.width - width) / 2;
  int right = left + width;

  int border = 3;
  // Draw border.
  DrawSquare(&tetris->drawer, {left - border, 0}, {left, game->window.height},
             Colors::kTeal);

  DrawSquare(&tetris->drawer, {right, 0}, {right + border, game->window.height},
             Colors::kTeal);

  // Draw the grid.
  for (int y = 1; y < (int)board->height; y++) {
    DrawSquare(&tetris->drawer, {left, y * side}, {right, y * side - 1},
               Colors::kGray);
  }

  for (int x = 1; x < (int)board->width; x++) {
    DrawSquare(&tetris->drawer,
               {left + x * side, 0},
               {left + x * side - 1, game->window.height},
               Colors::kGray);
  }
}


int BlockTypeToColor(uint8_t type) {
  static std::map<uint8_t, int> kColorMap = {
    {kLiveBlock, Colors::kBlue},
    {kDeadBlock, Colors::kRed},
  };

  auto it = kColorMap.find(type);
  if (it == kColorMap.end())
    return 0;
  return it->second;
}

void DrawBlock(Game* game, Tetris* tetris, int color, int x, int y) {
  // Generate the board.
  int side = game->window.height / 20;
  int width = side * tetris->board.width;
  int left_pad = (game->window.width - width) / 2;

  int ax = left_pad + x * side;
  int ay = game->window.height - y * side - side;
  DrawSquare(&tetris->drawer, {ax, ay}, {ax + side - 1, ay + side - 1}, color);
}

void DrawBoard(Game* game, Tetris* tetris) {
  Board* board = &tetris->board;
  for (int y = 0; y < (int)board->height; y++) {
    for (int x = 0; x < (int)board->width; x++) {
      uint8_t block_type = GetSquare(&tetris->board, x, y);
      int color = BlockTypeToColor(block_type);
      switch (block_type) {
        case 0:
        case kNone:
          continue;
        case kLiveBlock:
          DrawBlock(game, tetris, color, x, y);
          continue;
        case kDeadBlock:
          DrawBlock(game, tetris, color, x, y);
          continue;
        default:
          NOT_REACHED("Invalid square type");
      }
    }
  }
}

static inline int CreateColor(uint8_t i) {
  return 0xff000000 | i << 16 | i << 8 | i;
}

void DrawDebugSquares(Game* game, Tetris* tetris) {
  Board* board = &tetris->board;
  for (int i = 0; i < board->height; i++) {
    DrawBlock(game, tetris, CreateColor(i * 10), 0, i);
  }
}

}  // namespace

RenderCommand TetrisEndFrame(Game* game, Tetris* tetris) {
  DrawBackground(game, tetris);
  DrawBoard(game, tetris);

  DrawDebugSquares(game, tetris);

  return DrawerEndFrame(&tetris->drawer);
}

// Utils -----------------------------------------------------------------------


}  // namespace tetris


