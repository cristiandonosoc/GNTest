// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/ui/imgui/imgui_renderer.h"

#include "warhol/ui/imgui/def.h"
#include "warhol/ui/imgui/imgui.h"
#include "warhol/ui/imgui/imgui_shaders.h"
#include "warhol/utils/assert.h"
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
  // A imgui vertex is 8 floats (32 bytes).
  // 512 kb of buffer is 16384 vertices, which seems reasonable.
  // The same amount of indices will use 4k, so we'll give it a bit more.
  // This number can be revisited if imgui uses more vertices.
  Mesh imgui_mesh;
  imgui_mesh.uuid = GetNextMeshUUID();
  imgui_mesh.attributes = {
    {2, AttributeType::kFloat},     // Pos.
    {2, AttributeType::kFloat},     // UV.
    {4, AttributeType::kUint8},     // Color.
  };

  imgui_mesh.vertex_size = AttributesSize(&imgui_mesh);
  InitMeshPools(&imgui_mesh, KILOBYTES(512), KILOBYTES(16));

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

}  // namespace imgui
}  // namespace warhol
