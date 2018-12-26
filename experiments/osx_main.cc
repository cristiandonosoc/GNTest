// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <warhol/graphics/GL/def.h>
#include <warhol/sdl2/sdl_context.h>
#include <warhol/utils/log.h>

using namespace warhol;

int main() {
  SDLContext sdl_context;
  if (!sdl_context.Init())
    exit(1);

  /* if (!GL::Init()) { */
  /*   LOG(DEBUG) << "Could not initialize GL subsystem"; */
  /*   exit(1); */
  /* } */

  GL::Init();
  LOG(INFO) << std::endl
            << "OpenGL Vendor: " << glGetString(GL_VENDOR) << std::endl
            << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl
            << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl
            << "OpenGL Shading Language Version: "
            << glGetString(GL_SHADING_LANGUAGE_VERSION);


}
