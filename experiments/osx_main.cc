// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <warhol/graphics/GL/def.h>
#include <warhol/imgui/imgui_context.h>
#include <warhol/sdl2/sdl_context.h>
#include <warhol/utils/log.h>
#include <warhol/utils/location.h>


using namespace warhol;

volatile bool a = true;

void Baz() {
  SCOPE_LOCATION();
  if (a)
    Baz();
}

void Bar() {
  SCOPE_LOCATION();
  Baz();
}

void Foo() {
  SCOPE_LOCATION();
  Bar();
}

int main() {
  SCOPE_LOCATION();
  Foo();
  SDLContext sdl_context;
  if (!sdl_context.Init())
    exit(1);

  GL::Init();
  LOG(INFO) << std::endl
            << "OpenGL Vendor: " << glGetString(GL_VENDOR) << std::endl
            << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl
            << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl
            << "OpenGL Shading Language Version: "
            << glGetString(GL_SHADING_LANGUAGE_VERSION);

  ImguiContext imgui_context;
  if (!imgui_context.Init()) {
    LOG(ERROR) << "Could not initialize ImguiContext. Exiting.";
    exit(1);
  }



}
