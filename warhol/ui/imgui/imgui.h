// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "warhol/scene/camera.h"
#include "warhol/ui/imgui/imgui_renderer.h"
#include "warhol/utils/macros.h"

struct ImGuiIO;

namespace warhol {

struct InputState;
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

  Camera camera;
  ImguiRenderer imgui_renderer;
};

inline bool Valid(ImguiContext* imgui) {
  return !!imgui->io && Valid(&imgui->imgui_renderer);
}

// Both the renderer and window must outlive the imgui context.
bool InitImgui(Renderer* renderer, ImguiContext*);
void ShutdownImgui(ImguiContext*);

void ImguiNewFrame(Window*, InputState*, ImguiContext*);
RenderCommand ImguiGetRenderCommand(ImguiContext*);

}  // namespace imgui
}  // namespace warhol
