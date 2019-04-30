// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "tetris_renderer.h"

#include <map>

#include "game.h"
#include "tetris.h"

namespace tetris {

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
          NOT_REACHED() << "Invalid square type";
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

RenderCommand GetTetrisRenderCommand(Game* game, Tetris* tetris) {
  DrawBackground(game, tetris);
  DrawBoard(game, tetris);

  DrawDebugSquares(game, tetris);

  return DrawerEndFrame(&tetris->drawer);
}



}  // namespace tetris
