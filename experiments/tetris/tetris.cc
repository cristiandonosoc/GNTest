// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "tetris.h"

#include <stdlib.h>

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

int CoordToPos(Board* board, size_t x, size_t y) {
  ASSERT(x < board->width);
  ASSERT(y < board->height);
  return y * board->width + x;
}

int CoordToPos(Board* board, Int2 coord) {
  return CoordToPos(board, coord.x, coord.y);
}

void ClearLiveShape(Tetris* tetris) {
  // We clear the live blocks.
  Board* board = &tetris->board;
  for (size_t y = 0; y < board->height; y++) {
    for (size_t x = 0; x < board->width; x++) {
      size_t index = CoordToPos(board, x, y);
      ASSERT(index < board->slots.size());
      if (board->slots[index] == kLiveBlock)
        board->slots[index] = 0;
    }
  }
}

bool InBounds(Board* board, size_t x, size_t y) {
  return x < board->width && y < board->height;
}

bool InBounds(Board* board, Int2 coord) {
  return InBounds(board, coord.x, coord.y);
}

void PlaceCurrentShape(Tetris* tetris) {
  if (!HasCurrentShape(tetris))
    return;

  Int2 pos = tetris->current_shape.pos;
  Board* board = &tetris->board;
  board->slots[CoordToPos(board, tetris->current_shape.pos)] = kLiveBlock;
  for (auto& offset : tetris->current_shape.shape.offsets) {
    auto new_pos = pos + offset;
    if (InBounds(board, new_pos))
      board->slots[CoordToPos(board, new_pos)] = kLiveBlock;
  }
}

#define PASSED_LIMIT(last, tick, limit) (last + tick < limit)

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

void UpdateSideMove(Game* game, Tetris* tetris) {
  if (tetris->last_side_move + tetris->side_move_tick > game->time.seconds)
    return;
  tetris->last_side_move = game->time.seconds;

  // Left trumps right (down with capitalism!).
  auto& pos = tetris->current_shape.pos;
  if (game->input.left) {
    pos.x--;
  } else {
    pos.x++;
  }

  pos.x = Clamp(pos.x, 0, (int)tetris->board.width - 1);
}

void UpdateCurrentShape(Game* game, Tetris* tetris) {
  if (game->input.left || game->input.right)
    UpdateSideMove(game, tetris);

  // Move the shape down anyway.
  if (tetris->last_move_time + tetris->move_tick > game->time.seconds) {
    return;
  }

  tetris->last_move_time = game->time.seconds;
  tetris->current_shape.pos.y--;
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
  (void)type;
  return Colors::kBlue;
}

void DrawBlock(Game* game, Tetris* tetris, uint8_t type, int x, int y) {
  // Generate the board.
  int side = game->window.height / 20;
  int width = side * tetris->board.width;
  int left_pad = (game->window.width - width) / 2;

  int color = BlockTypeToColor(type);

  int ax = left_pad + x * side;
  int ay = game->window.height - y * side;
  DrawSquare(&tetris->drawer, {ax, ay}, {ax + side - 1, ay + side - 1}, color);
}

void DrawBoard(Game* game, Tetris* tetris) {
  Board* board = &tetris->board;
  for (int y = 0; y < (int)board->height; y++) {
    for (int x = 0; x < (int)board->width; x++) {
      uint8_t square = GetSquare(tetris, x, y);

      switch (square) {
        case kLiveBlock:
          DrawBlock(game, tetris, square, x, y);
        case 0:
          continue;
        default:
          NOT_REACHED("Invalid square type");
      }
    }
  }
}

}  // namespace

uint8_t GetSquare(Tetris* tetris, size_t x, size_t y) {
  ASSERT(x < tetris->board.width);
  ASSERT(y < tetris->board.height);
  return tetris->board.slots[CoordToPos(&tetris->board, x, y)];
}

RenderCommand TetrisEndFrame(Game* game, Tetris* tetris) {
  DrawBackground(game, tetris);
  DrawBoard(game, tetris);

  return DrawerEndFrame(&tetris->drawer);
}

}  // namespace tetris

