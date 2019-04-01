// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/ui/imgui/imgui.h"

#include <third_party/imgui/imgui.h>

#include "warhol/input/input.h"
#include "warhol/utils/assert.h"
#include "warhol/utils/log.h"
#include "warhol/window/common/window.h"

namespace warhol {
namespace imgui {

// Init ------------------------------------------------------------------------

namespace {

void MapIO(ImGuiIO* io) {
  // Keyboard mapping. ImGui will use those indices to peek into the
  // io.KeysDown[] array.
  io->KeyMap[ImGuiKey_Tab] = GET_KEY(Tab);
  io->KeyMap[ImGuiKey_LeftArrow] = GET_KEY(Left);
  io->KeyMap[ImGuiKey_RightArrow] = GET_KEY(Right);
  io->KeyMap[ImGuiKey_UpArrow] = GET_KEY(Up);
  io->KeyMap[ImGuiKey_DownArrow] = GET_KEY(Down);
  io->KeyMap[ImGuiKey_PageUp] = GET_KEY(PageUp);
  io->KeyMap[ImGuiKey_PageDown] = GET_KEY(PageDown);
  io->KeyMap[ImGuiKey_Home] = GET_KEY(Home);
  io->KeyMap[ImGuiKey_End] = GET_KEY(End);
  io->KeyMap[ImGuiKey_Insert] = GET_KEY(Insert);
  io->KeyMap[ImGuiKey_Delete] = GET_KEY(Delete);
  io->KeyMap[ImGuiKey_Backspace] = GET_KEY(Backspace);
  io->KeyMap[ImGuiKey_Space] = GET_KEY(Space);
  io->KeyMap[ImGuiKey_Enter] = GET_KEY(Enter);
  io->KeyMap[ImGuiKey_Escape] = GET_KEY(Escape);
  io->KeyMap[ImGuiKey_A] = GET_KEY(A);
  io->KeyMap[ImGuiKey_C] = GET_KEY(C);
  io->KeyMap[ImGuiKey_V] = GET_KEY(V);
  io->KeyMap[ImGuiKey_X] = GET_KEY(X);
  io->KeyMap[ImGuiKey_Y] = GET_KEY(Y);
  io->KeyMap[ImGuiKey_Z] = GET_KEY(Z);
}

}  // namespace

bool InitImgui(Renderer* renderer, ImguiContext* imgui) {
  if (Valid(imgui)) {
    LOG(ERROR) << "Imgui context already initialized.";
    return false;
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  imgui->io = &ImGui::GetIO();
  ASSERT(imgui->io);

  if (InitImguiRenderer(renderer, &imgui->renderer))
    return false;

  MapIO(imgui->io);
  return true;
}

// Shutdown --------------------------------------------------------------------

ImguiContext::~ImguiContext() {
  if (Valid(this))
    ShutdownImgui(this);
}

void ShutdownImgui(ImguiContext* imgui) {
  ASSERT(!Valid(imgui));

  if (Valid(&imgui->renderer))
    ShutdownImguiRenderer(&imgui->renderer);

  ImGui::DestroyContext();
}

// New Frame -------------------------------------------------------------------


namespace {

void RestartKeys(Window* window, InputState* input, ImGuiIO* io) {
  for (size_t i = 0; i < ARRAY_SIZE(io->KeysDown); i++) {
    io->KeysDown[i] = false;
  }


  for (size_t i = 0; i < InputState::kInputSize; i++) {
    if (input->keys_down[i])
      io->KeysDown[i] = true;
  }

  io->KeyCtrl = input->keys_down[GET_KEY(Ctrl)];
  io->KeyAlt = input->keys_down[GET_KEY(Alt)];
  io->KeyShift = input->keys_down[GET_KEY(Shift)];
  io->KeySuper = input->keys_down[GET_KEY(Super)];

  // Pass in the text input characters.
  io->AddInputCharactersUTF8(window->utf8_chars_inputted);

  // Update Mouse.
  io->MousePos = { (float)input->mouse.pos.x, (float)input->mouse.pos.y };
  io->MouseDown[0] = input->mouse.left;
  io->MouseDown[1] = input->mouse.right;
  io->MouseDown[2] = input->mouse.middle;

  // TODO(Cristian): Update cursors.
}


}  // namespace


void ImguiNewFrame(Window* window, InputState* input, ImguiContext* imgui) {
  ASSERT(Valid(window));
  ASSERT(Valid(imgui));

  imgui->io->DisplaySize = {(float)window->width, (float)window->height};

  // TODO(Cristian): Obtain time delta from platform!
  imgui->io->DeltaTime = window->frame_delta;

  RestartKeys(window, input, imgui->io);

  ImGui::NewFrame();
}

// Get RenderCommand -----------------------------------------------------------

RenderCommand ImguiGetRenderCommand(ImguiContext*);

}  // namespace imgui
}  // namespace warhol
