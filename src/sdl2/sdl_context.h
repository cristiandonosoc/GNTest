// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <memory>

#include "src/input/input.h"
#include "src/utils/macros.h"
#include "src/utils/status.h"

// TODO(Cristian): See if we can forward declare SDL_GLContext and thus avoid
//                 the whole SDLContextImpl.
struct SDL_Window;

namespace warhol {

struct InputState;
struct SDLContextImpl;

class SDLContext {
 public:
  enum class EventAction {
    kContinue,
    kQuit,
  };

  SDLContext();
  ~SDLContext();

  // If init is not success, the instance should not be used and should be
  // destroyed.
  Status Init();
  void Clear();

  bool valid() const { return impl_ != nullptr; }

  // Can be null if !valid().
  SDL_Window* get_window() const;

  int width() const;
  int height() const;

  // Returns the seconds since Init() was called. This is a fractional number.
  float GetSeconds() const;

  // TODO(Cristian): Perhaps later we're going to need an array of actions.
  //                 This also should be separated from SDL.
  EventAction HandleInputAndEvents(InputState*);

 private:
  std::unique_ptr<SDLContextImpl> impl_;

  DELETE_COPY_AND_ASSIGN(SDLContext);
};

}  // namespace warhol
