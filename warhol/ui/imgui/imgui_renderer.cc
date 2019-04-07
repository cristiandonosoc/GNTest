// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/ui/imgui/imgui_renderer.h"

#include "warhol/ui/imgui/def.h"
#include "warhol/ui/imgui/imgui.h"
#include "warhol/ui/imgui/imgui_shaders.h"
#include "warhol/utils/assert.h"
#include "warhol/utils/glm_impl.h"
#include "warhol/utils/log.h"
#include "warhol/graphics/common/mesh.h"
#include "warhol/graphics/common/renderer.h"

namespace warhol {
namespace imgui {

// Init ------------------------------------------------------------------------

namespace {

bool CreateShader(Renderer* renderer, ImguiRenderer* imgui) {
  SCOPE_LOCATION();
  Shader shader;
  switch (renderer->type) {
    case RendererType::kOpenGL:
      shader = GetOpenGLImguiShader();
      break;
    case RendererType::kVulkan:
    case RendererType::kLast:
      LOG(ERROR) << "Unsupported renderer type: " << ToString(renderer->type);
      return false;
  }

  if (!Loaded(&shader) || !RendererStageShader(renderer, &shader))
    return false;

  UnloadShader(&shader);
  imgui->shader = std::move(shader);
  return true;
}

bool CreateMesh(Renderer* renderer, ImguiRenderer* imgui) {
  SCOPE_LOCATION();
  // Create a Mesh for creating a buffer.
  Mesh imgui_mesh;
  imgui_mesh.name = "Imgui Mesh";
  imgui_mesh.uuid = GetNextMeshUUID();
  imgui_mesh.attributes = {
    {2, AttributeType::kFloat, false},  // Pos.
    {2, AttributeType::kFloat, false},  // UV.
    {4, AttributeType::kUint8, true},   // Color.
  };

  // A imgui vertex is 20 bytes. An index is 4 bytes.
  // 512 kb / 20 = 26214 vertices.
  // 512 kb / 4 = 131072 indices.
  imgui_mesh.vertex_size = AttributesSize(&imgui_mesh);
  InitMeshPools(&imgui_mesh, KILOBYTES(512), KILOBYTES(512));

  if (!RendererStageMesh(renderer, &imgui_mesh))
    return false;


  imgui->mesh = std::move(imgui_mesh);
  return true;
}

bool CreateFontTexture(Renderer* renderer, ImguiRenderer* imgui) {
  SCOPE_LOCATION();
  ASSERT(imgui->io);

  // IMGUI AUTHOR NOTE:
  // Load as RGBA 32-bits (75% of the memory is wasted, but default font is
  // so small) because it is more likely to be compatible with user's existing
  // shaders. If your ImTextureId represent a higher-level concept than just a
  // GL texture id, consider calling GetTexDataAsAlpha8() instead to save on
  // GPU memory.
  uint8_t* pixels;
  int width, height;
  imgui->io->Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

  Texture texture;
  texture.uuid = GetNextTextureUUID();
  texture.type = TextureType::kOpenGL;
  texture.x = width;
  texture.y = height;
  texture.channels = 4;
  texture.data = pixels;

  if (!RendererStageTexture(renderer, &texture))
    return false;

  // Imgui wants a way of tracking the font texture id to relay it back to use
  // on render time.
  imgui->io->Fonts->TexID = (ImTextureID)(uintptr_t)texture.uuid.value;
  imgui->font_texture = std::move(texture);
  return true;
}

}  // namespace

bool InitImguiRenderer(ImguiRenderer* imgui_renderer, Renderer* renderer) {
  ASSERT(!Valid(imgui_renderer));
  SCOPE_LOCATION();

  InitMemoryPool(&imgui_renderer->memory_pool, KILOBYTES(64));

  if (!CreateShader(renderer, imgui_renderer) ||
      !CreateMesh(renderer, imgui_renderer) ||
      !CreateFontTexture(renderer, imgui_renderer)) {
    return false;
  }

  imgui_renderer->renderer = renderer;

  return true;
}

// Shutdown --------------------------------------------------------------------

namespace {

void UnstageMesh(ImguiRenderer* imgui) {
  RendererUnstageMesh(imgui->renderer, &imgui->mesh);
  imgui->mesh = {};
}

void UnstageShader(ImguiRenderer* imgui) {
  RendererUnstageShader(imgui->renderer, &imgui->shader);
  imgui->shader = {};
}

void UnstageFonts(ImguiRenderer* imgui) {
  RendererUnstageTexture(imgui->renderer, &imgui->font_texture);
  UnloadTexture(&imgui->font_texture);
}

}  // namespace

void ShutdownImguiRenderer(ImguiRenderer* imgui) {
  ASSERT(Valid(imgui));
  UnstageMesh(imgui);
  UnstageShader(imgui);
  UnstageFonts(imgui);

  imgui->io = nullptr;
  imgui->renderer = nullptr;
  ShutdownMemoryPool(&imgui->memory_pool);
}

ImguiRenderer::~ImguiRenderer() {
  if (Valid(this))
    ShutdownImguiRenderer(this);
}

// GetRenderCommand ------------------------------------------------------------

namespace {
void ResetMesh(Mesh* mesh) {
  mesh->vertex_count = 0;
  mesh->index_count = 0;
  ResetMemoryPool(&mesh->vertices);
  ResetMemoryPool(&mesh->indices);
}

};

RenderCommand ImguiGetRenderCommand(ImguiRenderer* imgui_renderer) {
  /* ASSERT(Valid(imgui)); */
  /* ASSERT(Valid(&imgui->imgui_renderer)); */
  ASSERT(Valid(imgui_renderer));

  // Reset the memory pools wher ethe new index data is going to be.
  ResetMemoryPool(&imgui_renderer->memory_pool);
  ResetMesh(&imgui_renderer->mesh);

  ImGuiIO* io = imgui_renderer->io;
  ImDrawData* draw_data = ImGui::GetDrawData();

  // Avoid rendering when minimized, scale coordinates for retina displays
  // (screen coordinates != framebuffer coordinates)
  int fb_width =
      (int)(draw_data->DisplaySize.x * io->DisplayFramebufferScale.x);
  int fb_height =
      (int)(draw_data->DisplaySize.y * io->DisplayFramebufferScale.y);
  if (fb_width <= 0 || fb_height <= 0)
    return {};

  imgui_renderer->camera.viewport_p1 = {0, 0};
  imgui_renderer->camera.viewport_p2 = {fb_width, fb_height};

  float L = draw_data->DisplayPos.x;
  float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
  float T = draw_data->DisplayPos.y;
  float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
  imgui_renderer->camera.projection = glm::ortho(L, R, B, T);

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
      render_action.mesh = &imgui_renderer->mesh;
      render_action.textures = &imgui_renderer->font_texture;

      PushVertices(&imgui_renderer->mesh, cmd_list->VtxBuffer.Data,
                                         cmd_list->VtxBuffer.Size);
      PushIndices(&imgui_renderer->mesh, cmd_list->IdxBuffer.Data,
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
                                 &imgui_renderer->memory_pool,
                                 std::move(render_action));

      index_buffer_offset += draw_cmd->ElemCount * sizeof(uint32_t);
    }

    // We stage the buffers to the renderer.
    if (!RendererUploadMeshRange(imgui_renderer->renderer,
                                 &imgui_renderer->mesh)) {
      NOT_REACHED("Could not upload data to the renderer.");
    }
  }

  RenderCommand render_command;
  render_command.name = "Imgui";
  render_command.type = RenderCommandType::kMesh;
  render_command.config.blend_enabled = true;
  render_command.config.cull_faces = false;
  render_command.config.depth_test = false;
  render_command.config.scissor_test = true;
  render_command.camera = &imgui_renderer->camera;
  render_command.shader = &imgui_renderer->shader;
  render_command.actions.mesh_actions = std::move(mesh_actions);

  return render_command;
}

}  // namespace imgui
}  // namespace warhol
