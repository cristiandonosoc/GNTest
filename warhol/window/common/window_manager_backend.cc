// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/window/common/window_manager_backend.h"

#include <unordered_map>

#include "warhol/utils/assert.h"
#include "warhol/utils/log.h"

namespace warhol {

// WindowEvent -----------------------------------------------------------------

const char* WindowEvent::TypeToString(WindowEvent::Type event) {
  switch (event) {
    case WindowEvent::Type::kQuit: return "Quit";
    case WindowEvent::Type::kWindowResize: return "Window Resize";
    case WindowEvent::Type::kLast: return "Last";
  }

  NOT_REACHED("Unknown WindowEvent.");
  return nullptr;
}

// WindowManagerBackend Suscription / Getting ----------------------------------

namespace {

using FactoryMap =
    std::unordered_map<WindowManagerBackend::Type, WindowManagerBackendFactory>;

FactoryMap* GetFactoryMap() {
  static FactoryMap factory_map;
  return &factory_map;
}

}  // namespace

void SuscribeWindowManagerBackendFactory(WindowManagerBackend::Type type,
                                         WindowManagerBackendFactory factory) {
  LOG(DEBUG) << "Suscribing Window Manager Backend: "
             << WindowManagerBackend::TypeToString(type);
  FactoryMap* factory_map = GetFactoryMap();
  ASSERT(factory_map->find(type) == factory_map->end());
  factory_map->insert({type, factory});
}

std::unique_ptr<WindowManagerBackend>
CreateWindowManagerBackend(WindowManagerBackend::Type type) {
  FactoryMap* factory_map = GetFactoryMap();
  auto it = factory_map->find(type);
  ASSERT(it != factory_map->end());

  WindowManagerBackendFactory factory = it->second;
  return factory();
}

// WindowManagerBackend Interface ----------------------------------------------


WindowManagerBackend::WindowManagerBackend() = default;
WindowManagerBackend::WindowManagerBackend(Type type) : type(type) {}
WindowManagerBackend::~WindowManagerBackend() = default;

std::vector<const char*>
WindowManagerBackend::GetVulkanInstanceExtensions() {
  NOT_REACHED("This function must be subclassed.");
  return {};
}

bool WindowManagerBackend::CreateVulkanSurface(void*, void*) {
  NOT_REACHED("This function must be subclassed.");
  return false;
}

const char* WindowManagerBackend::TypeToString(Type type) {
  switch (type) {
    case WindowManagerBackend::Type::kSDLVulkan: return "SDL Vulkan";
    case WindowManagerBackend::Type::kLast: return "Last";
  }

  NOT_REACHED("Unknown WindowManagerBackend::Type.");
  return nullptr;
}

}  // namespace warhol
