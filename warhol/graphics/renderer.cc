// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/renderer.h"

#include <unordered_map>

#include "warhol/utils/assert.h"
#include "warhol/utils/log.h"
#include "warhol/graphics/common/renderer_backend.h"
#include "warhol/graphics/common/shader.h"
#include "warhol/window/window_manager.h"

namespace warhol {

namespace {

void Reset(Renderer* renderer) {
  renderer->window = nullptr;
  renderer->backend.reset();
  renderer->render_commands.clear();
}

using FactoryMap =
    std::unordered_map<RendererType, RendererBackendFactoryFunction>;

FactoryMap* GetFactoryMap() {
  static FactoryMap factory_map;
  return &factory_map;
}

std::unique_ptr<RendererBackend>
CreateRendererBackend(RendererType type) {
  FactoryMap* factory_map = GetFactoryMap();
  auto it = factory_map->find(type);
  ASSERT(it != factory_map->end());

  RendererBackendFactoryFunction factory = it->second;
  return factory();
}

}  // namespace

void SuscribeRendererBackendFactory(RendererType type,
                                    RendererBackendFactoryFunction factory) {
  LOG(DEBUG) << "Suscribing Renderer Backend: " << ToString(type);

  FactoryMap* factory_map = GetFactoryMap();
  ASSERT(factory_map->find(type) == factory_map->end());
  factory_map->insert({type, factory});
}

// Renderer --------------------------------------------------------------------

Renderer::~Renderer() {
  if (Valid(this))
    ShutdownRenderer(this);
}

void InitRenderer(Renderer* renderer, RendererType type) {
  ASSERT(type != RendererType::kLast);

  renderer->backend = CreateRendererBackend(type);
  renderer->backend->Init(renderer);
}

// An null backend renderer can happen if Shutdown was called before the
// destructor.
void ShutdownRenderer(Renderer* renderer) {
  ASSERT(Valid(renderer));
  renderer->backend->Shutdown();
  Reset(renderer);
}

// void WindowSizeChanged(Renderer* renderer, uint32_t width, uint32_t height) {}

bool RendererStageShader(Renderer* renderer, Shader* shader) {
  ASSERT(Valid(renderer));

  if (shader->uuid == 0) {
    LOG(ERROR) << "Staging shader with UUID 0.";
    return false;
  }

  if (!HasSource(shader)) {
    LOG(ERROR) << "Attempting to load shader without sources (name: "
               << shader->name << ").";
    return false;
  }

  return renderer->backend->StageShader(shader);
};

bool RendererUnstageShader(Renderer* renderer, Shader* shader) {
  ASSERT(Valid(renderer));
  renderer->backend->UnstageShader(shader);
};

void RendererStartFrame(Renderer* renderer) {
  ASSERT(Valid(renderer));
  renderer->backend->StartFrame(renderer);

}

void RendererExecuteCommands(Renderer* renderer,
                             LinkedList<RenderCommand>* commands) {
  ASSERT(Valid(renderer));
  renderer->backend->ExecuteCommands(renderer, commands);
}

void RendererEndFrame(Renderer* renderer) {
  ASSERT(Valid(renderer));
  renderer->backend->EndFrame(renderer);

}

}  // namespace warhol
