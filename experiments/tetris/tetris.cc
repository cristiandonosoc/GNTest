// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "tetris.h"

#include <stdlib.h>

#include "tetris_renderer.h"
#include "game.h"

namespace tetris {

bool HasTickTimerTriggered(Game* game, TickTimer* timer) {
  if (timer->last_tick + timer->length > game->time.seconds)
    return false;

  timer->last_tick = game->time.seconds;
  return true;
}

bool InitTetris(Game* game, Tetris* tetris) {
  Board board = {};
  board.width = 10;
  board.height = 20;
  uint32_t slot_count = board.width * board.height;
  board._slots.resize(slot_count);
  for (size_t i = 0; i < slot_count; i++) {
    board._slots[i] = 0;
  }

  for (int y = 0; y < 2; y++) {
    for (int x = 0; x < 8; x++) {
      SetSquare(&board, kDeadBlock, x, y);
    }
  }

  *tetris = {};
  tetris->board = std::move(board);

  return InitDrawer(&tetris->drawer, &game->renderer, &game->window);
}

// Update ----------------------------------------------------------------------

namespace {

const Shape kShapes[] = {
    // Square.
    {{{0, 0}, {-1, 0}, {-1, 1}, {0, 1}}},
    // L.
    {{{0, 0}, {-1, 0}, {-2, 0}, {0, 1}}},
    // Reverse-L.
    {{{0, 0}, {1, 0}, {2, 0}, {0, 1}}},
    // Squiggly.
    {{{0, 0}, {1, 0}, {1, 1}, {0, -1}}},
    // Reverse-Squiggly.
    {{{0, 0}, {-1, 0}, {-1, 1}, {0, -1}}},
};

enum class ShapeIndex {
  kSquare = 0,
  kL,
  kInverseL,
  kSquiggly,
  kReverseSquiggly,
  kLast,
};
static_assert(ARRAY_SIZE(kShapes) == (size_t)ShapeIndex::kLast);

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
      if (GetSquare(board, x, y) == kLiveBlock)
          SetSquare(board, 0, x, y);
    }
  }
}

void PlaceCurrentShape(Tetris* tetris) {
  if (!HasCurrentShape(tetris))
    return;

  Int2 pos = tetris->current_shape.pos;
  Board* board = &tetris->board;
  SetSquare(board, kLiveBlock, tetris->current_shape.pos);
  for (auto& offset : tetris->current_shape.shape.offsets) {
    auto new_pos = pos + offset;
    if (WithinBounds(board, new_pos))
    SetSquare(board, kLiveBlock, new_pos);
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
  if (!HasTickTimerTriggered(game, &tetris->next_shape_tick))
    return;

  // Create a new shape.
  Board* board = &tetris->board;
  int index = rand() % ARRAY_SIZE(kShapes);
  tetris->current_shape.shape = kShapes[index];
  Int2 new_pos;
  new_pos.y = board->height - 1;
  new_pos.x = rand() % board->width;
  tetris->current_shape.pos = new_pos;
  LOG(DEBUG) << "Creating new shape at: " << ToString(new_pos);
}

// Returns an offset of where the shape should move.
Int2 UpdateSideMove(Game* game, Tetris* tetris) {
  if (!game->input.left && !game->input.right)
    return {};

  if (!HasTickTimerTriggered(game, &tetris->side_move_tick))
    return {};

  // Left trumps right (down with capitalism!).
  if (game->input.left)
    return {-1, 0};
  else
    return {1, 0};
}

Int2 UpdateDownMovement(Game* game, Tetris* tetris) {
  if (!game->input.down)
    return {};

  if (!HasTickTimerTriggered(game, &tetris->down_move_tick))
    return {};

  // Because we move down explicitly, we reset the auto move down timer.
  tetris->auto_down_tick.last_tick = game->time.seconds;
  return {0, -1};
}

Int2 UpdateAutoDownMovement(Game* game, Tetris* tetris) {
  if (!HasTickTimerTriggered(game, &tetris->auto_down_tick))
    return {};
  return {0, -1};
}

bool IsRowComplete(Board* board, int y) {
  for (int x = 0; x < board->width; x++) {
    uint8_t square = GetSquare(board, x, y);
    if (square != kDeadBlock)
      return false;
  }
  return true;
}

void RemoveRow(Board* board, int y) {
  for (int x = 0; x < board->width; x++) {
    SetSquare(board, kNone, x, y);
  }
}

void MoveBlocksDown(Board* board, int y) {
  for (; y < board->height; y++) {
    for (int x = 0; x < board->width; x++) {
      uint8_t square = GetSquare(board, x, y);
      if (square != kDeadBlock || y == 0)
        continue;

      // We set the square below this as dead and clear the top one.
      SetSquare(board, kDeadBlock, x, y - 1);
      SetSquare(board, kNone, x, y);
    }
  }
}

void LookForCompletedRows(Board* board) {
  // We look bottom up for a complete row.
  int y = 0;
  while (y < board->height) {
    if (!IsRowComplete(board, y)) {
      y++;
      continue;
    }

    RemoveRow(board, y);
    MoveBlocksDown(board, y);

    // We moved the blocks down, so we need to recheck this row again.
    continue;
  }
}

void DoShapeCollision(Tetris* tetris, Int2) {
  // We cristalize the shape.
  for (Int2& sqr_offset : tetris->current_shape.shape.offsets) {
    Int2 sqr_pos = tetris->current_shape.pos + sqr_offset;
    SetSquare(&tetris->board, kDeadBlock, sqr_pos);
  }

  tetris->current_shape = {};

  // We check if we destroyed lines.
  LookForCompletedRows(&tetris->board);
}

void UpdateCurrentShape(Game* game, Tetris* tetris) {
  SCOPE_LOCATION();

  Int2 offset = {};
  if (game->input.down) {
    offset = UpdateDownMovement(game, tetris);
  } else {
    offset = UpdateAutoDownMovement(game, tetris);
  }

  if (!game->input.down) {
    offset += UpdateSideMove(game, tetris);
  }

  if (IsZero(offset))
    return;

  auto collision_type = CheckShapeCollision(&tetris->board,
                                            &tetris->current_shape.shape,
                                            tetris->current_shape.pos,
                                            offset);

  switch (collision_type) {
    case CollisionType::kNone:
      LOG(DEBUG) << "Before: " << ToString(tetris->current_shape.pos);
      tetris->current_shape.pos += offset;
      LOG(DEBUG) << "After: " << ToString(tetris->current_shape.pos);
      return;
    case CollisionType::kBorder:
      return;
    case CollisionType::kBottom:
    case CollisionType::kShape:
      DoShapeCollision(tetris, offset);
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

RenderCommand TetrisEndFrame(Game* game, Tetris* tetris) {
  return GetTetrisRenderCommand(game, tetris);
}

}  // namespace tetris
