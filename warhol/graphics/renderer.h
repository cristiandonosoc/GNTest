// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdlib.h>

#include <memory>
#include <vector>

#include "warhol/utils/macros.h"

#include "warhol/graphics/common/shader_manager.h"
#include "warhol/graphics/common/render_command.h"

namespace warhol {

struct RendererBackend;   // Defined at end of file.
struct WindowManager;

// Backend Suscription ---------------------------------------------------------

enum class RendererType {
  kVulkan,
  kLast,  // DO NOT specialize with numbers.
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

  WindowManager* window = nullptr;
  std::unique_ptr<RendererBackend> backend = {};

  // Holds all the general view of loaded shaders/uniforms.
  ShaderManager shader_manager;
  std::vector<RenderCommand> render_commands;
};

inline bool Valid(Renderer* r) { return !!r->backend; }

bool InitRenderer(Renderer*, RendererType, WindowManager*);
// Will be called on destructor if the renderer is valid.
void ShutdownRenderer(Renderer*);

void WindowSizeChanged(Renderer*, uint32_t width, uint32_t height);

// Each render command has to come accompanied with all the appropiate uniform
// values.
//
// See warhol/graphics/common/render_command.h for more details.
void AddRenderCommand(RenderCommand*, UniformValue* values, size_t count);

void DrawFrame(Renderer*);

// RendererBackend -------------------------------------------------------------

struct RendererBackend {
  virtual ~RendererBackend();

  // Virtual interface.

  virtual bool Init(Renderer*) = 0;
  virtual void Shutdown() = 0;
  virtual void ExecuteCommands(RenderCommand*, size_t command_count) = 0;
  virtual void DrawFrame(Camera*) = 0;

  // Loads the mesh into the GPU.
  virtual void LoadMesh(Mesh*) = 0;
  virtual void UnloadMesh(Mesh*) = 0;

  virtual ShaderManager* GetShaderManager() = 0;
};

}  // namespace
