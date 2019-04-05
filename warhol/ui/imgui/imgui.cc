// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/ui/imgui/imgui.h"

#include <third_party/imgui/imgui.h>

#include "warhol/utils/glm_impl.h"

#include "warhol/graphics/common/renderer.h"
#include "warhol/graphics/common/render_command.h"
#include "warhol/input/input.h"
#include "warhol/utils/assert.h"
#include "warhol/utils/log.h"
#include "warhol/window/common/window.h"

// Imgui config verification check.
// This is to verify that out imgui config wasn't overwritten by a imgui update.
// Our imgui config is in warhol/ui/imgui/warhol_imgui_config.h
// That config should be placed in third_party/imgui/imconfig.h
#ifndef WARHOL_IMGUI_CONFIG
#error No warhol imgui config loaded. Is third_party/imgui/imconfig.h correct?
#endif

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
  SCOPE_LOCATION();
  if (Valid(imgui)) {
    LOG(ERROR) << "Imgui context already initialized.";
    return false;
  }

  IMGUI_CHECKVERSION();

  imgui->imgui_renderer.io = imgui->io;
  if (!InitImguiRenderer(renderer, &imgui->imgui_renderer))
    return false;

  ImGui::CreateContext();
  imgui->io = &ImGui::GetIO();
  ASSERT(imgui->io);
  MapIO(imgui->io);

  imgui->imgui_renderer.camera.projection = glm::mat4(1.0f);
  imgui->imgui_renderer.camera.view = glm::mat4(1.0f);

  return true;
}

// Shutdown --------------------------------------------------------------------

ImguiContext::~ImguiContext() {
  if (Valid(this))
    ShutdownImgui(this);
}

void ShutdownImgui(ImguiContext* imgui) {
  ASSERT(Valid(imgui));

  if (Valid(&imgui->imgui_renderer))
    ShutdownImguiRenderer(&imgui->imgui_renderer);

  ImGui::DestroyContext();
}

// Start Frame -----------------------------------------------------------------

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


void ImguiStartFrame(Window* window, InputState* input, ImguiContext* imgui) {
  ASSERT(Valid(window));
  ASSERT(Valid(imgui));

  imgui->io->DisplaySize = {(float)window->width, (float)window->height};

  // TODO(Cristian): Obtain time delta from platform!
  imgui->io->DeltaTime = window->frame_delta;

  RestartKeys(window, input, imgui->io);

  ImGui::NewFrame();
}

// End Frame -------------------------------------------------------------------

void ImguiEndFrame(ImguiContext* imgui) {
  ASSERT(Valid(imgui));
  // Will finalize the draw data needed for getting the draw lists for getting
  // the render command.
  ImGui::Render();
}

// Get RenderCommand -----------------------------------------------------------

RenderCommand ImguiGetRenderCommand(ImguiContext* imgui) {
  ASSERT(Valid(imgui));

  // Reset the memory pools wher ethe new index data is going to be.
  auto& imgui_renderer = imgui->imgui_renderer;
  ResetMemoryPool(&imgui_renderer.memory_pool);
  ResetMemoryPool(&imgui_renderer.mesh.vertices);
  ResetMemoryPool(&imgui_renderer.mesh.indices);

  ImGuiIO* io = imgui->io;
  ImDrawData* draw_data = ImGui::GetDrawData();

  // Avoid rendering when minimized, scale coordinates for retina displays
  // (screen coordinates != framebuffer coordinates)
  int fb_width =
      (int)(draw_data->DisplaySize.x * io->DisplayFramebufferScale.x);
  int fb_height =
      (int)(draw_data->DisplaySize.y * io->DisplayFramebufferScale.y);
  if (fb_width <= 0 || fb_height <= 0)
    return {};

  imgui_renderer.camera.viewport_p1 = {0, 0};
  imgui_renderer.camera.viewport_p2 = {fb_width, fb_height};

  float L = draw_data->DisplayPos.x;
  float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
  float T = draw_data->DisplayPos.y;
  float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
  imgui_renderer.camera.projection = glm::ortho(L, R, B, T);

  // Represents how much in the indices mesh buffer we're in.
  uint64_t index_buffer_offset = 0;

  LinkedList<MeshRenderAction> mesh_actions;

  // Create the draw list.
  ImVec2 pos = draw_data->DisplayPos;
  for (int i = 0; i < draw_data->CmdListsCount; i++) {
    ImDrawList* cmd_list = draw_data->CmdLists[i];

    // This will start appending drawing data into the mesh buffer that's
    // already staged into the renderer.
    for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
      const ImDrawCmd* draw_cmd = &cmd_list->CmdBuffer[cmd_i];

      MeshRenderAction render_action;
      render_action.mesh = &imgui->imgui_renderer.mesh;
      render_action.textures = &imgui->imgui_renderer.font_texture;

      PushVertices(&imgui_renderer.mesh, cmd_list->VtxBuffer.Data,
                                         cmd_list->VtxBuffer.Size);
      PushIndices(&imgui_renderer.mesh, cmd_list->IdxBuffer.Data,
                                        cmd_list->IdxBuffer.Size);

      IndexRange range = 0;
      range = PushOffset(range, index_buffer_offset);
      range = PushSize(range, draw_cmd->ElemCount);
      render_action.index_range = range;

      // We check if we need to apply scissoring.
      Vec4 scissor_rect;
      scissor_rect.x = draw_cmd->ClipRect.x - pos.x;
      scissor_rect.y = draw_cmd->ClipRect.y - pos.y;
      scissor_rect.z = draw_cmd->ClipRect.z - pos.x;
      scissor_rect.w = draw_cmd->ClipRect.w - pos.y;
      if (scissor_rect.x < fb_width && scissor_rect.y < fb_height &&
          scissor_rect.z >= 0.0f && scissor_rect.w >= 0.0f) {
        render_action.scissor = scissor_rect;
      }

      PushIntoListFromMemoryPool(&mesh_actions,
                                 &imgui_renderer.memory_pool,
                                 std::move(render_action));

      index_buffer_offset += draw_cmd->ElemCount;
    }

    // We stage the buffers to the renderer.
    if (!RendererUploadMeshRange(imgui_renderer.renderer,
                                 &imgui_renderer.mesh)) {
      NOT_REACHED("Could not upload data to the renderer.");
    }
  }

  RenderCommand render_command;
  render_command.type = RenderCommandType::kMesh;
  render_command.config.blend_enabled = true;
  render_command.config.cull_faces = false;
  render_command.config.depth_test = false;
  render_command.camera = &imgui_renderer.camera;
  render_command.actions.mesh_actions = std::move(mesh_actions);

  return render_command;
}

}  // namespace imgui
}  // namespace warhol
