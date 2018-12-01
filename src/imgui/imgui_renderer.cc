// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/imgui/imgui_renderer.h"

#include <third_party/imgui/imgui.h>

#include "src/utils/macros.h"
#include "src/assets.h"
#include "src/utils/file.h"
#include "src/utils/log.h"

namespace warhol {

namespace {

// TODO(Cristian): Improve this stupid ass interface.
std::tuple<bool, std::vector<char>, std::vector<char>>
ReadShaders(const char* vert_file, const char* frag_file) {
  std::vector<char> vert;
  if (!ReadWholeFile(Assets::ShaderPath(vert_file), &vert))
    return {false, {}, {}};

  std::vector<char> frag;
  if (!ReadWholeFile(Assets::ShaderPath(frag_file), &frag))
    return {false, {}, {}};

  return {true, std::move(vert), std::move(frag)};
}

}  // namespace

ImguiRenderer::ImguiRenderer() = default;

ImguiRenderer::~ImguiRenderer() {
  if (vbo_)
    glDeleteBuffers(1, &vbo_);
  if (ebo_)
    glDeleteBuffers(1, &ebo_);

  if (font_texture_) {
    glDeleteTextures(1, &font_texture_);
    // TODO(Cristian): We're leaking state here...
    /* io_->Fonts->TexID = 0; */
  }
}

bool ImguiRenderer::Init(ImGuiIO* io) {
  if (auto [ok, vert_src, frag_src] = ReadShaders("imgui.vert", "imgui.frag");
      ok) {
    shader_ = Shader("imgui", vert_src.data(), frag_src.data());
    if (!shader_.Init())
      return false;
  } else {
    return false;
  }

  LOG(DEBUG) << "Created imgui renderer shader";
  for (auto [key, attrib] : shader_.attributes()) {
    LOG(DEBUG) << key << ": " << attrib.location;
  }

  CreateFontTexture(io);

  // Generate the buffers
  GLuint buf[2];
  glGenBuffers(ARRAY_SIZE(buf), buf);
  vbo_ = buf[0];
  ebo_ = buf[1];

  init_ = true;
  return true;
}

void
ImguiRenderer::CreateFontTexture(ImGuiIO* io) {
  // Build texture atlas
  unsigned char* pixels;
  int width, height;
  io->Fonts->GetTexDataAsRGBA32(
      &pixels, &width, &height);  // Load as RGBA 32-bits (75% of the memory
                                  // is wasted, but default font is so small)
                                  // because it is more likely to be
                                  // compatible with user's existing shaders.
                                  // If your ImTextureId represent a
                                  // higher-level concept than just a GL
                                  // texture id, consider calling
                                  // GetTexDataAsAlpha8() instead to save on
                                  // GPU memory.

  // Upload texture to graphics system
  /* GLint last_texture; */
  /* glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture); */

  // TODO(Cristian): See how we can use a texture to support this.
  glGenTextures(1, &font_texture_);
  glBindTexture(GL_TEXTURE_2D, font_texture_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glTexImage2D(GL_TEXTURE_2D,
               0,
               GL_RGBA,
               width,
               height,
               0,
               GL_RGBA,
               GL_UNSIGNED_BYTE,
               pixels);

  // Store our identifier
  io->Fonts->TexID = (ImTextureID)(intptr_t)font_texture_;
}

void
ImguiRenderer::Render(ImGuiIO* io, ImDrawData* draw_data) {
  assert(init_);
  // Avoid rendering when minimized, scale coordinates for retina displays
  // (screen coordinates != framebuffer coordinates)
  int fb_width =
      (int)(draw_data->DisplaySize.x * io->DisplayFramebufferScale.x);
  int fb_height =
      (int)(draw_data->DisplaySize.y * io->DisplayFramebufferScale.y);
  if (fb_width <= 0 || fb_height <= 0)
    return;
  draw_data->ScaleClipRects(io->DisplayFramebufferScale);

  // Backup GL state
  GLenum last_active_texture;
  glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&last_active_texture);
  glActiveTexture(GL_TEXTURE0);
  GLint last_program;
  glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
  GLint last_texture;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
#ifdef GL_SAMPLER_BINDING
  GLint last_sampler;
  glGetIntegerv(GL_SAMPLER_BINDING, &last_sampler);
#endif
  GLint last_array_buffer;
  glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
  GLint last_vertex_array;
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
#ifdef GL_POLYGON_MODE
  GLint last_polygon_mode[2];
  glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
#endif
  GLint last_viewport[4];
  glGetIntegerv(GL_VIEWPORT, last_viewport);
  GLint last_scissor_box[4];
  glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
  GLenum last_blend_src_rgb;
  glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb);
  GLenum last_blend_dst_rgb;
  glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb);
  GLenum last_blend_src_alpha;
  glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha);
  GLenum last_blend_dst_alpha;
  glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha);
  GLenum last_blend_equation_rgb;
  glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb);
  GLenum last_blend_equation_alpha;
  glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha);
  GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
  GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
  GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
  GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);

  // Setup render state: alpha-blending enabled, no face culling, no depth
  // testing, scissor enabled, polygon fill
  glEnable(GL_BLEND);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_SCISSOR_TEST);
#ifdef GL_POLYGON_MODE
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif

  // Setup viewport, orthographic projection matrix
  // Our visible imgui space lies from draw_data->DisplayPos (top left) to
  // draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayMin is
  // typically (0,0) for single viewport apps.
  glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
  float L = draw_data->DisplayPos.x;
  float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
  float T = draw_data->DisplayPos.y;
  float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
  const float ortho_projection[4][4] = {
      {2.0f / (R - L), 0.0f, 0.0f, 0.0f},
      {0.0f, 2.0f / (T - B), 0.0f, 0.0f},
      {0.0f, 0.0f, -1.0f, 0.0f},
      {(R + L) / (L - R), (T + B) / (B - T), 0.0f, 1.0f},
  };
  shader_.Use();
  /* glUniform1i(g_AttribLocationTex, 0); */
  assert(shader_.SetInt(Shader::Uniform::kTexSampler0, 0));
  auto* u_proj = shader_.GetUniform(Shader::Uniform::kProjection);
  assert(u_proj);
  glUniformMatrix4fv(u_proj->location, 1, GL_FALSE, &ortho_projection[0][0]);
#ifdef GL_SAMPLER_BINDING
  glBindSampler(0, 0);  // We use combined texture/sampler state. Applications
                        // using GL 3.3 may set that otherwise.
#endif
  // Recreate the VAO every time
  // (This is to easily allow multiple GL contexts. VAO are not shared among GL
  // contexts, and we don't track creation/deletion of windows so we don't have
  // an obvious key to use to cache them.)
  GLuint vao_handle = 0;
  glGenVertexArrays(1, &vao_handle);
  glBindVertexArray(vao_handle);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);

  const Shader::Attribute* attribute;
  attribute = shader_.GetAttribute(Shader::Attribute::kPos);
  assert(attribute );
  glVertexAttribPointer(attribute ->location,
                        2,
                        GL_FLOAT,
                        GL_FALSE,
                        sizeof(ImDrawVert),
                        (GLvoid*)IM_OFFSETOF(ImDrawVert, pos));
  glEnableVertexAttribArray(attribute ->location);

  attribute = shader_.GetAttribute(Shader::Attribute::kTexCoord0);
  assert(attribute);
  glVertexAttribPointer(attribute ->location,
                        2,
                        GL_FLOAT,
                        GL_FALSE,
                        sizeof(ImDrawVert),
                        (GLvoid*)IM_OFFSETOF(ImDrawVert, uv));
  glEnableVertexAttribArray(attribute ->location);

  attribute = shader_.GetAttribute(Shader::Attribute::kColor);
  assert(attribute );
  glVertexAttribPointer(attribute ->location,
                        4,
                        GL_UNSIGNED_BYTE,
                        GL_TRUE,
                        sizeof(ImDrawVert),
                        (GLvoid*)IM_OFFSETOF(ImDrawVert, col));
  glEnableVertexAttribArray(attribute ->location);

  // Draw
  ImVec2 pos = draw_data->DisplayPos;
  for (int n = 0; n < draw_data->CmdListsCount; n++) {
    const ImDrawList* cmd_list = draw_data->CmdLists[n];
    const ImDrawIdx* idx_buffer_offset = 0;

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert),
                 (const GLvoid*)cmd_list->VtxBuffer.Data,
                 GL_STREAM_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx),
                 (const GLvoid*)cmd_list->IdxBuffer.Data,
                 GL_STREAM_DRAW);

    for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
      const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
      if (pcmd->UserCallback) {
        // User callback (registered via ImDrawList::AddCallback)
        pcmd->UserCallback(cmd_list, pcmd);
      } else {
        ImVec4 clip_rect = ImVec4(pcmd->ClipRect.x - pos.x,
                                  pcmd->ClipRect.y - pos.y,
                                  pcmd->ClipRect.z - pos.x,
                                  pcmd->ClipRect.w - pos.y);
        if (clip_rect.x < fb_width && clip_rect.y < fb_height &&
            clip_rect.z >= 0.0f && clip_rect.w >= 0.0f) {
          // Apply scissor/clipping rectangle
          glScissor((int)clip_rect.x,
                    (int)(fb_height - clip_rect.w),
                    (int)(clip_rect.z - clip_rect.x),
                    (int)(clip_rect.w - clip_rect.y));

          // Bind texture, Draw
          glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
          glDrawElements(
              GL_TRIANGLES,
              (GLsizei)pcmd->ElemCount,
              sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
              idx_buffer_offset);
        }
      }
      idx_buffer_offset += pcmd->ElemCount;
    }
  }
  glDeleteVertexArrays(1, &vao_handle);

  // Restore modified GL state
  glUseProgram(last_program);
  glBindTexture(GL_TEXTURE_2D, last_texture);
#ifdef GL_SAMPLER_BINDING
  glBindSampler(0, last_sampler);
#endif
  glActiveTexture(last_active_texture);
  glBindVertexArray(last_vertex_array);
  glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
  glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
  glBlendFuncSeparate(last_blend_src_rgb,
                      last_blend_dst_rgb,
                      last_blend_src_alpha,
                      last_blend_dst_alpha);
  if (last_enable_blend)
    glEnable(GL_BLEND);
  else
    glDisable(GL_BLEND);
  if (last_enable_cull_face)
    glEnable(GL_CULL_FACE);
  else
    glDisable(GL_CULL_FACE);
  if (last_enable_depth_test)
    glEnable(GL_DEPTH_TEST);
  else
    glDisable(GL_DEPTH_TEST);
  if (last_enable_scissor_test)
    glEnable(GL_SCISSOR_TEST);
  else
    glDisable(GL_SCISSOR_TEST);
#ifdef GL_POLYGON_MODE
  glPolygonMode(GL_FRONT_AND_BACK, (GLenum)last_polygon_mode[0]);
#endif
  glViewport(last_viewport[0],
             last_viewport[1],
             (GLsizei)last_viewport[2],
             (GLsizei)last_viewport[3]);
  glScissor(last_scissor_box[0],
            last_scissor_box[1],
            (GLsizei)last_scissor_box[2],
            (GLsizei)last_scissor_box[3]);
}

}  // namespace warhol
