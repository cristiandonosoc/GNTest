// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdlib.h>

#include <memory>
#include <vector>

#include "warhol/utils/macros.h"

#include "warhol/graphics/common/render_command.h"
#include "warhol/graphics/common/renderer_backend.h"

#include "warhol/math/vec.h"

namespace warhol {

struct Shader;
struct Window;

// Renderers are implemented in terms of backends (OpenGL, Vulkan, etc.).
// This means that the renderer is a simple proxy object the users are warhol
// will use, while the functionality is actually implemented by the |backend|
// object within it.

// Backend Suscription ---------------------------------------------------------

enum class RendererType {
  kOpenGL,
  kVulkan,
  kLast,
};
const char* ToString(RendererType);

// Each backend, upon application startup, must suscribe a function that will
// be called to create a that particular RendererBackend.
using RendererBackendFactoryFunction = std::unique_ptr<RendererBackend> (*)();
void SuscribeRendererBackendFactory(RendererType,
                                    RendererBackendFactoryFunction);

// Renderer --------------------------------------------------------------------

struct Renderer {
  ~Renderer();  // "RAII" semantics.

  Vec3 clear_color;

  Window* window = nullptr;
  std::unique_ptr<RendererBackend> backend;

  std::vector<RenderCommand> render_commands;
};

inline bool Valid(Renderer* r) { return !!r->backend; }

bool InitRenderer(Renderer*, RendererType, Window*);
// Will be called on destructor if the renderer is valid.
void ShutdownRenderer(Renderer*);

void WindowSizeChanged(Renderer*, uint32_t width, uint32_t height);

// Each render command has to come accompanied with all the appropiate uniform
// values.
//
// See warhol/graphics/common/render_command.h for more details.
void AddRenderCommand(RenderCommand*, UniformValue* values, size_t count);

// Resource Uploading.

bool RendererStageMesh(Renderer*, Mesh*);
void RendererUnstageMesh(Renderer*, Mesh*);

bool RendererStageShader(Renderer*, Shader*);
void RendererUnstageShader(Renderer*, Shader*);

bool RendererStageTexture(Renderer*, Texture*);
void RendererUnstageTexture(Renderer*, Texture*);

void RendererStartFrame(Renderer*);
void RendererExecuteCommands(Renderer*, LinkedList<RenderCommand>* commands);
void RendererEndFrame(Renderer*);

}  // namespace
