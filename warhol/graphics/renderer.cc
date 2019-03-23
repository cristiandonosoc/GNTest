// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/renderer.h"

#include <unordered_map>

#include "warhol/graphics/common/renderer_backend.h"
#include "warhol/utils/assert.h"
#include "warhol/utils/log.h"
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

void DrawFrame(Renderer* renderer, Camera* camera) {
  ASSERT(Valid(renderer));
  renderer->backend->DrawFrame(camera);
}

}  // namespace warhol
