// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

namespace warhol {

struct StageTextureConfig;
struct Texture;

namespace opengl {

struct OpenGLRendererBackend;

bool OpenGLStageTexture(OpenGLRendererBackend*, Texture*, StageTextureConfig*);

void OpenGLUnstageTexture(OpenGLRendererBackend*, Texture*);

}  // namespace opengl
}  // namespace warhol
