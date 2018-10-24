// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/imgui/imgui_context.h"

#include <SDL2/SDL.h>
#include <third_party/imgui/imgui.h>

#include "src/input/input.h"

namespace warhol {

ImguiContext::ImguiContext(SDL_Window* window)
    : window_(window) {};

ImguiContext::~ImguiContext() {
  for (SDL_Cursor* cursor : cursors_)
    SDL_FreeCursor(cursor);
  ImGui::DestroyContext();
}

bool ImguiContext::Init() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  io_ = &ImGui::GetIO();

  // Init the renderer.
  if (!renderer_.Init(io_)) {
    LOG(ERROR) << "Could not initialize Imgui Renderer";
    return false;
  }

  // Keyboard mapping. ImGui will use those indices to peek into the
  // io.KeysDown[] array.
  io_->KeyMap[ImGuiKey_Tab] = GET_KEY(Tab);
  io_->KeyMap[ImGuiKey_LeftArrow] = GET_KEY(Left);
  io_->KeyMap[ImGuiKey_RightArrow] = GET_KEY(Right);
  io_->KeyMap[ImGuiKey_UpArrow] = GET_KEY(Up);
  io_->KeyMap[ImGuiKey_DownArrow] = GET_KEY(Down);
  io_->KeyMap[ImGuiKey_PageUp] = GET_KEY(PageUp);
  io_->KeyMap[ImGuiKey_PageDown] = GET_KEY(PageDown);
  io_->KeyMap[ImGuiKey_Home] = GET_KEY(Home);
  io_->KeyMap[ImGuiKey_End] = GET_KEY(End);
  io_->KeyMap[ImGuiKey_Insert] = GET_KEY(Insert);
  io_->KeyMap[ImGuiKey_Delete] = GET_KEY(Delete);
  io_->KeyMap[ImGuiKey_Backspace] = GET_KEY(Backspace);
  io_->KeyMap[ImGuiKey_Space] = GET_KEY(Space);
  io_->KeyMap[ImGuiKey_Enter] = GET_KEY(Enter);
  io_->KeyMap[ImGuiKey_Escape] = GET_KEY(Escape);
  io_->KeyMap[ImGuiKey_A] = GET_KEY(A);
  io_->KeyMap[ImGuiKey_C] = GET_KEY(C);
  io_->KeyMap[ImGuiKey_V] = GET_KEY(V);
  io_->KeyMap[ImGuiKey_X] = GET_KEY(X);
  io_->KeyMap[ImGuiKey_Y] = GET_KEY(Y);
  io_->KeyMap[ImGuiKey_Z] = GET_KEY(Z);

  // Cursors
  // TODO(Cristian): Check for cursor errors.
  cursors_.resize(ImGuiMouseCursor_COUNT);
  cursors_[ImGuiMouseCursor_Arrow] =
      SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
  cursors_[ImGuiMouseCursor_TextInput] =
      SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
  cursors_[ImGuiMouseCursor_ResizeAll] =
      SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
  cursors_[ImGuiMouseCursor_ResizeNS] =
      SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
  cursors_[ImGuiMouseCursor_ResizeEW] =
      SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
  cursors_[ImGuiMouseCursor_ResizeNESW] =
      SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
  cursors_[ImGuiMouseCursor_ResizeNWSE] =
      SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
  cursors_[ImGuiMouseCursor_Hand] =
      SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);

  init_ = true;
  return true;
}

void
ImguiContext::NewFrame(InputState* input) {
  assert(init_);
  assert(io_->Fonts->IsBuilt());  // See imgui_impl_sdl.cpp
  // Setup display size (every frame to accommodate for window resizing)
  int w, h;
  int display_w, display_h;
  // TODO(Cristian): Get this through SDLContext?
  SDL_GetWindowSize(window_, &w, &h);
  SDL_GL_GetDrawableSize(window_, &display_w, &display_h);
  io_->DisplaySize = ImVec2((float)w, (float)h);
  io_->DisplayFramebufferScale = ImVec2(w > 0 ? ((float)display_w / w) : 0,
                                        h > 0 ? ((float)display_h / h) : 0);

  // Setup time step (we don't use SDL_GetTicks() because it is using
  // millisecond resolution)
  static Uint64 frequency = SDL_GetPerformanceFrequency();
  Uint64 current_time = SDL_GetPerformanceCounter();
  io_->DeltaTime = time_ > 0
                       ? (float)((double)(current_time - time_) / frequency)
                       : (float)(1.0f / 60.0f);
  time_ = current_time;

  // Update Keyboard.
  // We pass in the keys that are down.
  for (size_t i = 0; i < InputState::kInputSize; i++) {
    if (input->keys_down[i])
      io_->KeysDown[i] = true;
  }
  io_->KeyCtrl = input->keys_down[GET_KEY(Ctrl)];
  io_->KeyAlt = input->keys_down[GET_KEY(Alt)];
  io_->KeyShift = input->keys_down[GET_KEY(Shift)];
  io_->KeySuper = input->keys_down[GET_KEY(Super)];

  // Update Mouse.
  io_->MousePos = {(float)input->cur_mouse.x, (float)input->cur_mouse.y};
  io_->MouseDown[0] = input->cur_mouse.left;
  io_->MouseDown[1] = input->cur_mouse.right;
  io_->MouseDown[2] = input->cur_mouse.middle;

  // Hide or update mouse cursor.
  ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
  if (io_->MouseDrawCursor || imgui_cursor == ImGuiMouseCursor_None) {
    SDL_ShowCursor(SDL_FALSE);
  } else {
    assert((uint32_t)imgui_cursor < cursors_.size());
    // If the cursor is available, draw it. Otherwise show an arrow.
    SDL_SetCursor(cursors_[imgui_cursor] ? cursors_[imgui_cursor]
                                         : cursors_[ImGuiMouseCursor_Arrow]);
  }

  // Mark the frame as starting.
  ImGui::NewFrame();
}

void ImguiContext::Render() {
  assert(init_);
  // Construct all the draw orders.
  ImGui::Render();
  renderer_.Render(io_, ImGui::GetDrawData());
}

bool
ImguiContext::keyboard_captured() const {
  return io_->WantCaptureKeyboard;
}

bool
ImguiContext::mouse_captured() const {
  return io_->WantCaptureMouse;
}

}  // namespace warhol
