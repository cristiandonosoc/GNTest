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

  auto dims = GetTetrisScreenDimensions(game, tetris);
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
  };

  auto it = kColorMap.find(type);
  if (it == kColorMap.end())
    return 0;
  return it->second;
}

void DrawBlock(Game* game, Tetris* tetris, int color, int x, int y) {
  // Generate the board.
  auto dims = GetTetrisScreenDimensions(game, tetris);

  int ax = dims.screen_pad + x * dims.block_size;
  int ay = game->window.height - y * dims.block_size - dims.block_size;
  DrawSquare(&tetris->drawer,
             {ax, ay}, {ax + dims.block_size - 1, ay + dims.block_size - 1},
             color);
}

void DrawBoard(Game* game, Tetris* tetris) {
  Board* board = &tetris->board;
  for (int y = 0; y < (int)board->height; y++) {
    for (int x = 0; x < (int)board->width; x++) {
      uint8_t block_type = GetSquare(&tetris->board, x, y);
      int color = BlockTypeToColor(block_type);
      switch (block_type) {
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
