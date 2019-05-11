// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <vector>

#include "warhol/utils/macros.h"

// TODO(Cristian): This should be forward declared.
#include "warhol/imgui/imgui_renderer.h"

struct ImGuiIO;
struct SDL_Window;
struct SDL_Cursor;

namespace warhol {

struct InputState;
class SDLContext;

// TODOs:
// - Do clipboard.

// Class that wraps all the Imgui initialization/state management. It also does
// the translation from Warhol's input to imgui.
// Imgui calls are valid as long as the ImguiContext is.
//
// Though technically there could be many ImguiContexts, ImGuiIO is a singleton,
// so there should never be more than one ImguiContext.
class ImguiContext {
 public:
  ImguiContext();
  ~ImguiContext();

  bool Init();
  void NewFrame(const SDLContext&, InputState*);
  void Render();

  // Do not pass the keyboard/mouse to the underlying application.
  bool keyboard_captured() const;
  bool mouse_captured() const;
  ImGuiIO& io() { return *io_; }

 private:
  ImGuiIO* io_;           // Not owning. Must outlive.

  // TODO(Cristian): This should be a unique pointer.
  ImguiRenderer renderer_;
  std::vector<SDL_Cursor*> cursors_;
  uint64_t time_ = 0;
  bool init_ = false;

  DELETE_COPY_AND_ASSIGN(ImguiContext);
};

}  // namespace warhol
