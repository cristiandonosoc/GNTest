// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/common/renderer_backend.h"

#include <unordered_map>

#include "warhol/utils/assert.h"
#include "warhol/utils/log.h"

namespace warhol {

// RendererBackend Suscription / Getting ----------------------------------

namespace {

using FactoryMap =
    std::unordered_map<RendererBackend::Type, RendererBackendFactory>;

FactoryMap* GetFactoryMap() {
  static FactoryMap factory_map;
  return &factory_map;
}

}  // namespace

void SuscribeRendererBackendFactory(RendererBackend::Type type,
                                    RendererBackendFactory factory) {
  LOG(DEBUG) << "Suscribing Renderer Backend: "
             << RendererBackend::TypeToString(type);
  FactoryMap* factory_map = GetFactoryMap();
  ASSERT(factory_map->find(type) == factory_map->end());
  factory_map->insert({type, factory});
}

std::unique_ptr<RendererBackend>
CreateRendererBackend(RendererBackend::Type type) {
  FactoryMap* factory_map = GetFactoryMap();
  auto it = factory_map->find(type);
  ASSERT(it != factory_map->end());

  RendererBackendFactory factory = it->second;
  return factory();
}

// RendererBackend Interface ----------------------------------------------

RendererBackend::RendererBackend() = default;
RendererBackend::RendererBackend(Type type) : type(type) {}
RendererBackend::~RendererBackend() = default;

const char* RendererBackend::TypeToString(RendererBackend::Type type) {
  switch (type) {
    case RendererBackend::Type::kVulkan: return "Vulkan";
    case RendererBackend::Type::kLast: return "Last";
  }

  NOT_REACHED("Unknown RendererBackendType.");
  return nullptr;
}

}  // namespace warhol
