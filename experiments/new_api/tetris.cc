// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "tetris.h"

#include <stdlib.h>

#include "game.h"

namespace tetris {

bool InitTetris(Game* game, Tetris* tetris) {
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

void UpdateNewShape(Game* game, Tetris* tetris) {
  if (tetris->last_shape_time + tetris->next_shape_tick < game->time.seconds)
    return;

  // Create a new shape.
  tetris->current_shape.pos.x = rand() % Tetris::kWidth;
  tetris->current_shape.pos.y = Tetris::kHeight;
}

void UpdateCurrentShape(Game* game, Tetris* tetris) {
  if (tetris->last_move_time + tetris->move_tick > game->time.seconds) {
    tetris->last_move_time = game->time.seconds;
    tetris->current_shape.pos.y--;
  }
}

/* constexpr uint8_t kDeadBlock = 1;   // A block that is already stationed. */
constexpr uint8_t kLiveBlock = 2;   // A block of a current shape.

int CoordToPos(int x, int y) {
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

void PlaceCurrentShape(Tetris* tetris) {
  if (!HasCurrentShape(tetris))
    return;

  Pair<int> coord;

  for (int i = -1; i < ARRAY_SIZE(tetris->current_shape.shape->moves); i++) {
    if (i == -1) {
      coord = tetris->current_shape.pos;
    } else {
      coord = MoveCoord(tetris->current_shape.pos,
                        tetris->current_shape.shape->moves[i]);
    }

  }

  tetris->board[CoordToPos(tetris->current_shape.pos)] = kLiveBlock;
}

void UpdateBoard(Tetris* tetris) {
  ClearLiveShape(tetris);

  // We need to check for collision.
  PlaceCurrentShape(tetris);
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

RenderCommand TetrisEndFrame(Game* game, Tetris* tetris) {
  // Generate the board.
  int side = game->window.height / 20;

  int width = side * Tetris::kWidth;
  int left_pad = (game->window.width - width) / 2;

  DrawSquare(&tetris->drawer,
             {left_pad, 0},
             {left_pad + width, game->window.height},
             Colors::kTeal);

  return {};
}

}  // namespace tetris



