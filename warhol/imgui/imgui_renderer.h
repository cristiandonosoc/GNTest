// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "warhol/shader.h"
#include "warhol/utils/macros.h"

struct ImDrawData;
struct ImGuiIO;

namespace warhol {

// Class in charge of rendering the generated Imgui data. Will be called
// through ImguiContext.
//
// This should be an interface to implement the Imgui renderer with different
// backends.
class ImguiRenderer {
 public:
  ImguiRenderer();
  ~ImguiRenderer();
  DELETE_COPY_AND_ASSIGN(ImguiRenderer);

  bool Init(ImGuiIO*);
  void Render(ImGuiIO*, ImDrawData*);

 private:
  void CreateFontTexture(ImGuiIO*);

  Shader shader_;
  GLuint vbo_ = 0;
  GLuint ebo_ = 0;
  GLuint font_texture_ = 0;
  bool init_ = false;
};

}  // namespace warhol
