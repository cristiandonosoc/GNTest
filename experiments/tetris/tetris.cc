// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "tetris.h"

#include <stdlib.h>

#include "tetris_renderer.h"
#include "game.h"
#include "shape.h"

namespace tetris {

bool InitTetris(Game* game, Tetris* tetris) {
  Board board = {};
  board.width = 15;
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
    {"square",            {{0, 0}, {-1, 0}, {-1, 1}, {0, 1}}},
    {"l",                 {{0, 0}, {-1, 0}, {-2, 0}, {0, 1}}},
    {"reverse-l",         {{0, 0}, {1, 0}, {2, 0}, {0, 1}}},
    {"squiggly",          {{0, 0}, {1, 0}, {1, 1}, {0, -1}}},
    {"reverse-squiggly",  {{0, 0}, {-1, 0}, {-1, 1}, {0, -1}}},
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
      uint8_t square = GetSquare(board, x, y);
      if (square == kLiveBlock || square == kPivot)
          SetSquare(board, 0, x, y);
    }
  }
}

void PlaceCurrentShape(Tetris* tetris) {
  if (!HasCurrentShape(tetris))
    return;

  Int2 pos = tetris->current_shape.pos;
  Board* board = &tetris->board;
  for (auto& offset : tetris->current_shape.shape.rotated_offsets) {
    auto new_pos = pos + offset;
    if (WithinBounds(board, new_pos))
      SetSquare(board, kLiveBlock, new_pos);
  }

  SetSquare(board, kPivot, pos + tetris->current_shape.shape.pivot);
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
  new_pos.x = rand() % board->width;
  new_pos.y = board->height - 1;

  // We check that we're not crossing the x_bounds.
  IntBox2 bounds = GetShapeBoundingBox(&tetris->current_shape.shape, new_pos);
  LOG(DEBUG) << "New " << tetris->current_shape.shape.name << " at "
             << ToString(new_pos) << ", Bounds: " << ToString(bounds);
  if (bounds.min.x < 0) {
    new_pos.x += 0 - bounds.min.x;
  } else if (bounds.min.y >= board->width) {
    new_pos.x -= bounds.max.x - (board->width - 1);
  }

  LOG(DEBUG) << "Creating new shape at: " << ToString(new_pos);
  tetris->current_shape.pos = new_pos;
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
  if (tetris->no_down)
    return {};
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

void DoShapeCollision(Tetris* tetris) {
  // We cristalize the shape.
  for (Int2& sqr_offset : tetris->current_shape.shape.rotated_offsets) {
    Int2 sqr_pos = tetris->current_shape.pos + sqr_offset;
    SetSquare(&tetris->board, kDeadBlock, sqr_pos);
  }

  tetris->current_shape = {};

  // We check if we destroyed lines.
  LookForCompletedRows(&tetris->board);
}

void AttemptRotation(Tetris* tetris) {
  // Get the rotated tetris offsets.
  Shape* shape = &tetris->current_shape.shape;
  auto rotated_offsets = GetRotatedOffsets(shape, shape->rotation + 1);

  auto collision = CheckCollision(
      &tetris->board, tetris->current_shape.pos, rotated_offsets);
  // Check if a rotation is possible.
  if (collision.type != CollisionType::kNone)
    return;

  // A rotation was possible!
  shape->rotation++;
  shape->rotated_offsets = std::move(rotated_offsets);
}

void UpdateCurrentShape(Game* game, Tetris* tetris) {
  SCOPE_LOCATION();

  if (KeyDownThisFrame(&game->input, Key::kSpace))
    tetris->no_down = !tetris->no_down;

  if (KeyDownThisFrame(&game->input, Key::kUp))
    AttemptRotation(tetris);

  Int2 offset = {};
  if (game->input.down) {
    LOG(DEBUG) << "Updating down move.";
    offset = UpdateDownMovement(game, tetris);
  } else {
    offset = UpdateAutoDownMovement(game, tetris);
  }

  if (!game->input.down) {
    offset += UpdateSideMove(game, tetris);
  }

  if (IsZero(offset))
    return;

  auto collision = CheckShapeCollision(&tetris->board,
                                       &tetris->current_shape.shape,
                                       tetris->current_shape.pos + offset);
  if (collision.type != CollisionType::kNone) {
    LOG(DEBUG) << "Got collision: " << CollisionTypeToString(collision.type)
               << " at " << ToString(collision.pos);
  }

  switch (collision.type) {
    case CollisionType::kNone:
      tetris->current_shape.pos += offset;
      return;
    case CollisionType::kBorder:
      return;
    case CollisionType::kBottom:
      DoShapeCollision(tetris);
      return;
    case CollisionType::kShape:
      // If it was a side move, we simply bump against the shape.
      if (offset.x == 0)
        DoShapeCollision(tetris);
      return;
  }

  NOT_REACHED() << "Unknown collision type: " << (int)collision.type;
}

}  // namespace

void UpdateTetris(Game* game, Tetris* tetris) {
  DrawerNewFrame(&tetris->drawer);

  // We see if we need to createa a new shape.
  if (!HasCurrentShape(tetris)) {
    UpdateNewShape(game, tetris);
  } else {
    UpdateCurrentShape(game, tetris);
  }

  UpdateBoard(tetris);
}

// End Frame -------------------------------------------------------------------

RenderCommand TetrisEndFrame(Game* game, Tetris* tetris) {
  return GetTetrisRenderCommand(game, tetris);
}

// TickTimer -------------------------------------------------------------------

bool HasTickTimerTriggered(Game* game, TickTimer* timer) {
  if (timer->last_tick + timer->length > game->time.seconds)
    return false;

  timer->last_tick = game->time.seconds;
  return true;
}

float TimerRatio(Game* game, TickTimer* timer) {
  return (game->time.seconds - timer->last_tick) / timer->length;
}

}  // namespace tetris
