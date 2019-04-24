// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "tetris.h"

#include <stdlib.h>

#include "game.h"

namespace tetris {

bool InitTetris(Game* game, Tetris* tetris) {
  *tetris = {};
  return InitDrawer(&tetris->drawer, &game->renderer, &game->window);
}

// Update ----------------------------------------------------------------------

Shape Shapes::kSquare = {
    {Move::kUp, Move::kRight, Move::kDown},
};

namespace {

bool HasCurrentShape(Tetris* tetris) {
  return tetris->current_shape.shape;
}


int CoordToPos(size_t x, size_t y) {
  ASSERT(x < Tetris::kWidth);
  ASSERT(y < Tetris::kHeight);
  return y * Tetris::kWidth + x;
}

int CoordToPos(Pair<int> coord) {
  return CoordToPos(coord.x, coord.y);
}

void ClearLiveShape(Tetris* tetris) {
  // We clear the live blocks.
  for (int y = 0; y < Tetris::kHeight; y++) {
    for (int x = 0; x < Tetris::kWidth; x++) {
      int index = CoordToPos(x, y);
      ASSERT(index < ARRAY_SIZE(tetris->board));
      if (tetris->board[index] == kLiveBlock)
        tetris->board[index] = 0;
    }
  }
}

Pair<int> MoveCoord(Pair<int> coord, Move move) {
  switch (move) {
    case Move::kUp:
      return {coord.x, coord.y + 1};
    case Move::kDown:
      return {coord.x, coord.y - 1};
    case Move::kLeft:
      return {coord.x - 1, coord.y};
    case Move::kRight:
      return {coord.x + 1, coord.y};
  }

  NOT_REACHED("Invalid Move.");
  return {};
}

bool InBounds(Tetris* tetris, size_t x, size_t y) {
  return x < tetris->kWidth && y < tetris->kHeight;
}

void PlaceCurrentShape(Tetris* tetris) {
  if (!HasCurrentShape(tetris))
    return;

  Pair<int> coord;

  tetris->board[CoordToPos(tetris->current_shape.pos)] = kLiveBlock;
  for (int i = -1; i < ARRAY_SIZE(tetris->current_shape.shape->moves); i++) {
    if (i == -1) {
      coord = tetris->current_shape.pos;
    } else {
      coord = MoveCoord(tetris->current_shape.pos,
                        tetris->current_shape.shape->moves[i]);
    }

    if (InBounds(tetris, coord.x, coord.y))
      tetris->board[CoordToPos(coord)] = kLiveBlock;
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
  tetris->current_shape.shape = &Shapes::kSquare;
  tetris->current_shape.pos.x = rand() % Tetris::kWidth;
  tetris->current_shape.pos.y = Tetris::kHeight - 1;
  LOG(DEBUG) << "Creating new shape: " << tetris->current_shape.pos.ToString();
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

  pos.x = std::clamp(pos.x, 0, tetris->kWidth - 1);
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
  int side = game->window.height / 20;
  int width = side * Tetris::kWidth;
  int left = (game->window.width - width) / 2;
  int right = left + width;

  int border = 3;
  // Draw border.
  DrawSquare(&tetris->drawer, {left - border, 0}, {left, game->window.height},
             Colors::kTeal);

  DrawSquare(&tetris->drawer, {right, 0}, {right + border, game->window.height},
             Colors::kTeal);

  // Draw the grid.
  for (int y = 1; y < tetris->kHeight; y++) {
    DrawSquare(&tetris->drawer, {left, y * side}, {right, y * side - 1},
               Colors::kGray);
  }

  for (int x = 1; x < tetris->kWidth; x++) {
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
  int width = side * Tetris::kWidth;
  int left_pad = (game->window.width - width) / 2;

  int color = BlockTypeToColor(type);

  int ax = left_pad + x * side;
  int ay = game->window.height - y * side;
  DrawSquare(&tetris->drawer, {ax, ay}, {ax + side - 1, ay + side - 1}, color);
}

void DrawBoard(Game* game, Tetris* tetris) {
  for (int y = 0; y < Tetris::kHeight; y++) {
    for (int x = 0; x < Tetris::kWidth; x++) {
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
  ASSERT(x < Tetris::kWidth);
  ASSERT(y < Tetris::kHeight);
  return tetris->board[CoordToPos(x, y)];
}

RenderCommand TetrisEndFrame(Game* game, Tetris* tetris) {
  DrawBackground(game, tetris);
  DrawBoard(game, tetris);

  return DrawerEndFrame(&tetris->drawer);
}

}  // namespace tetris

