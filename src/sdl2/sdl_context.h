// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <memory>
#include <vector>

#include "src/input/input.h"
#include "src/utils/macros.h"
#include "src/utils/status.h"

// TODO(Cristian): See if we can forward declare SDL_GLContext and thus avoid
//                 the whole SDLContextImpl.
struct SDL_Window;
struct SDL_WindowEvent;

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
  // TODO(Cristian): Perhaps later we're going to need an array of actions.
  //                 EventAction should also be separated from SDL.
  EventAction NewFrame(InputState*);

  bool valid() const { return impl_ != nullptr; }
  // Can be null if !valid().
  SDL_Window* get_window() const;
  int width() const;
  int height() const;
  // If not empty, always will be a null-terminated string.
  const std::vector<char>& utf8_input() const { return utf8_chars_inputted_; }

  // Delta of time within the last frame in seconds.
  double frame_delta() const;
  // A rolling average of many frames.
  double frame_delta_average() const;
  // 1.0 / frame_delta_average()
  double framerate() const;

 private:
  void CalculateFramerate();
  void HandleWindowEvent(const SDL_WindowEvent&);

  std::unique_ptr<SDLContextImpl> impl_;
  // Chars received through the SDL_TEXTINPUT events during a frame.
  // Will be cleared at the beginning of calling NewFrame.
  // The string inside the vector will be null-terminated.
  std::vector<char> utf8_chars_inputted_;

  DELETE_COPY_AND_ASSIGN(SDLContext);
};

}  // namespace warhol
