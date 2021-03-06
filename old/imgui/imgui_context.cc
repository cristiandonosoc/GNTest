// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/imgui/imgui_context.h"

#include <third_party/imgui/imgui.h>

#include "warhol/input/input.h"
#include "warhol/sdl2/def.h"
#include "warhol/sdl2/sdl_context.h"
#include "warhol/utils/assert.h"
#include "warhol/utils/log.h"

namespace warhol {

ImguiContext::ImguiContext() = default;

ImguiContext::~ImguiContext() {
  for (SDL_Cursor* cursor : cursors_)
    SDL_FreeCursor(cursor);
  ImGui::DestroyContext();
}

bool ImguiContext::Init() {
  SCOPE_LOCATION();
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  io_ = &ImGui::GetIO();

  // Init the renderer.
  if (!renderer_.Init(io_)) {
    LOG(ERROR) << "Could not initialize Imgui Renderer";
    return false;
  }

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
ImguiContext::NewFrame(const SDLContext& sdl_context, InputState* input) {
  ASSERT(init_);
  ASSERT(io_->Fonts->IsBuilt());  // See imgui_impl_sdl.cpp

  // Setup display size (every frame to accommodate for window resizing)
  /* int w, h; */
  /* int display_w, display_h; */
  /* SDL_GetWindowSize(window_, &w, &h); */
  /* SDL_GL_GetDrawableSize(window_, &display_w, &display_h); */
  /* io_->DisplaySize = ImVec2((float)w, (float)h); */
  /* io_->DisplayFramebufferScale = ImVec2(w > 0 ? ((float)display_w / w) : 0, */
  /*                                       h > 0 ? ((float)display_h / h) : 0); */
  io_->DisplaySize = {(float)sdl_context.width(), (float)sdl_context.height()};
  // TODO(donosoc): Do we need to revisit this?
  io_->DisplayFramebufferScale =  { 1.0f, 1.0f };

  // Setup time step (we don't use SDL_GetTicks() because it is using
  // millisecond resolution)
  static Uint64 frequency = SDL_GetPerformanceFrequency();
  Uint64 current_time = SDL_GetPerformanceCounter();
  io_->DeltaTime = time_ > 0
                       ? (float)((double)(current_time - time_) / frequency)
                       : (float)(1.0f / 60.0f);
  time_ = current_time;

  // Restart the keys.
  for (bool& key_down : io_->KeysDown)
    key_down = false;

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

  // Pass in the text input characters.
  const auto& input_str_vec = sdl_context.utf8_input();
  if (!input_str_vec.empty())
    io_->AddInputCharactersUTF8(input_str_vec.data());

  // Update Mouse.
  io_->MousePos = { (float)input->mouse.pos.x, (float)input->mouse.pos.y };
  io_->MouseDown[0] = input->mouse.left;
  io_->MouseDown[1] = input->mouse.right;
  io_->MouseDown[2] = input->mouse.middle;

  // Hide or update mouse cursor.
  ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
  if (io_->MouseDrawCursor || imgui_cursor == ImGuiMouseCursor_None) {
    SDL_ShowCursor(SDL_FALSE);
  } else {
    ASSERT((uint32_t)imgui_cursor < cursors_.size());
    // If the cursor is available, draw it. Otherwise show an arrow.
    SDL_SetCursor(cursors_[imgui_cursor] ? cursors_[imgui_cursor]
                                         : cursors_[ImGuiMouseCursor_Arrow]);
  }

  // Mark the frame as starting.
  ImGui::NewFrame();
}

void ImguiContext::Render() {
  ASSERT(init_);
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
