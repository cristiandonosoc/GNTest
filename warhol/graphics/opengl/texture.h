// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

namespace warhol {

struct Texture;

namespace opengl {

struct OpenGLRendererBackend;

bool OpenGLStageTexture(OpenGLRendererBackend*, Texture*);

void OpenGLUnstageTexture(OpenGLRendererBackend*, Texture*);

}  // namespace opengl
}  // namespace warhol
