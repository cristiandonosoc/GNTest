// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "warhol/ui/imgui/imgui_renderer.h"
#include "warhol/utils/macros.h"

struct ImGuiIO;

namespace warhol {

struct InputState;
struct PlatformTime;
struct Renderer;
struct RenderCommand;
struct Window;

namespace imgui {

struct ImguiContext {
  RAII_CONSTRUCTORS(ImguiContext);

  // This struct represents a handle to the imgui system.
  // Not owning, must outlive.
  ImGuiIO* io = nullptr;

  bool keyboard_captured = false;
  bool mouse_captured = false;

  ImguiRenderer imgui_renderer;
};

inline bool Valid(ImguiContext* imgui) {
  return !!imgui->io && Valid(&imgui->imgui_renderer);
}

// Both the renderer and window must outlive the imgui context.
bool InitImgui(Renderer* renderer, ImguiContext*);
void ShutdownImgui(ImguiContext*);

void ImguiStartFrame(Window*, PlatformTime*, InputState*, ImguiContext*);

// Gets the command to be passed down to the renderer.
// IMPORTANT: StartFrame *has* to be called each frame before this.
RenderCommand ImguiEndFrame(ImguiContext*);

}  // namespace imgui
}  // namespace warhol
