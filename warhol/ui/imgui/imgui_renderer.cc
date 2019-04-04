// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/ui/imgui/imgui_renderer.h"

#include "warhol/utils/assert.h"
#include "warhol/utils/log.h"
#include "warhol/graphics/common/mesh.h"
#include "warhol/graphics/common/renderer.h"

namespace warhol {
namespace imgui {

// Init ------------------------------------------------------------------------

namespace {

#ifdef WARHOL_OPENGL_ENABLED

const char kOpenGLVertex[] = R"(
#version 330 core

// Attributes ------------------------------------------------------------------

precision mediump float;

layout (location = 0) in vec2 in_pos;
layout (location = 2) in vec2 in_uv;
layout (location = 1) in vec4 in_color;

out vec4 color;
out vec2 uv;

// Uniforms --------------------------------------------------------------------

layout (std140) uniform Camera {
  mat4 proj;
  mat4 view;
} camera;

void main() {
  gl_Position = camera.proj * camera.view * vec4(in_pos.xy, 0, 1.0f);
  color = in_color;
  uv = in_uv;
}
)";

const char kOpenGLFragment[] = R"(
#version 330 core

precision mediump float;

uniform sampler2D u_tex_sampler0;

in vec3 color;
in vec2 uv;

layout (location = 0) out vec4 out_color;

void main() {
  out_color = frag_color * texture(u_tex_sampler0, frag_uv);
}
)";

Shader GetOpenGLImguiShader() {
  Shader shader;
  shader.name = "Imgui Shader";
  shader.vert_ubo_size = 0;
  shader.frag_ubo_size = 0;
  shader.texture_count = 1;

  std::vector<uint8_t> src;

  size_t vert_size = sizeof(kOpenGLVertex);
  src.reserve(vert_size);
  src.insert(src.end(), kOpenGLVertex, kOpenGLVertex + vert_size);
  shader.vert_source = src;

  size_t frag_size = sizeof(kOpenGLFragment);
  src.reserve(frag_size);
  src.insert(src.end(), kOpenGLFragment, kOpenGLFragment+ frag_size);
  shader.frag_source = src;

  shader.uuid = GetNextShaderUUID();

  return shader;
}

#else
Shader GetOpenGLImguiShader() {
  NOT_REACHED("OpenGL support not enabled.");
  return {};
}
#endif

bool CreateShader(Renderer* renderer, ImguiRenderer* imgui) {
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

bool CreateBuffer(Renderer* renderer, ImguiRenderer* imgui) {
  // Create a Mesh for creating a buffer.
  // A imgui vertex is 8 floats (32 bytes).
  // 512 kb of buffer is 16384 vertices, which seems reasonable.
  // The same amount of indices will use 4k, so we'll give it a bit more.
  // This number can be revisited if imgui uses more vertices.
  Mesh imgui_mesh;
  InitMeshPools(&imgui_mesh, KILOBYTES(512), KILOBYTES(16));

  if (!RendererStageMesh(renderer, &imgui_mesh))
    return false;

  imgui->mesh = std::move(imgui_mesh);
  return true;
}

bool CreateFontTexture(Renderer* renderer, ImguiRenderer* imgui) {
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

bool InitImguiRenderer(Renderer* renderer, ImguiRenderer* imgui) {
  ASSERT(imgui->io);

  if (!CreateShader(renderer, imgui) ||
      !CreateBuffer(renderer, imgui) ||
      !CreateFontTexture(renderer, imgui)) {
    return false;
  }
  return true;
}

// Shutdown --------------------------------------------------------------------

ImguiRenderer::~ImguiRenderer() {
  if (Valid(this))
    ShutdownImguiRenderer(this);
}

void ShutdownImguiRenderer(ImguiRenderer* imgui) {
  ASSERT(Valid(imgui));
  RendererUnstageMesh(imgui->renderer, &imgui->mesh);
  RendererUnstageShader(imgui->renderer, &imgui->shader);
  RendererUnstageTexture(imgui->renderer, &imgui->font_texture);
}

}  // namespace imgui
}  // namespace warhol
