// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/window/window_manager.h"

#include <unordered_map>

#include "warhol/utils/assert.h"
#include "warhol/utils/log.h"

namespace warhol {

WindowManager::~WindowManager() {
  if (Valid(this))
    ShutdownWindowManager(this);
}

// Backend Suscription ---------------------------------------------------------

namespace {

using FactoryMap =
    std::unordered_map<WindowBackendType, WindowBackendFactoryFunction>;

FactoryMap* GetFactoryMap() {
  static FactoryMap factory_map;
  return &factory_map;
}

std::unique_ptr<WindowManagerBackend>
CreateWindowManagerBackend(WindowBackendType type) {
  FactoryMap* factory_map = GetFactoryMap();
  auto it = factory_map->find(type);
  ASSERT(it != factory_map->end());

  WindowBackendFactoryFunction factory = it->second;
  return factory();
}

}  // namespace

void SuscribeWindowManagerBackendFactory(WindowBackendType type,
                                         WindowBackendFactoryFunction factory) {
  LOG(DEBUG) << "Suscribing Window Manager Backend: " << ToString(type);

  FactoryMap* factory_map = GetFactoryMap();
  ASSERT(factory_map->find(type) == factory_map->end());
  factory_map->insert({type, factory});
}

// InitWindowManager -----------------------------------------------------------

namespace {

void Reset(WindowManager* window) {
  window->backend_type = WindowBackendType::kLast;
  window->backend.reset();
}

}  // namespace

bool InitWindowManager(WindowManager* window, WindowBackendType type) {
  ASSERT(type != WindowBackendType::kLast);

  window->backend = CreateWindowManagerBackend(type);
  bool success = window->backend->Init(window);
  if (!success)
    Reset(window);
  return success;
}

void ShutdownWindowManager(WindowManager* window) {
  ASSERT(Valid(window));
  window->backend->Shutdown();
  Reset(window);
}

std::pair<WindowEvent*, size_t>
UpdateWindowManager(WindowManager* window, InputState* input) {
  ASSERT(Valid(window));
  return window->backend->NewFrame(input);
}

std::vector<const char*>
WindowManagerGetVulkanInstanceExtensions(WindowManager* window) {
  ASSERT(Valid(window));
  return window->backend->GetVulkanInstanceExtensions();
}

// |vk_instance| & |surface_khr| must be casted to the right type in the
// implementation. This is so that we don't need to forward declare vulkan
// typedefs.
bool WindowManagerCreateVulkanSurface(WindowManager* window, void* vk_instance,
                                      void* surface_khr) {
  ASSERT(Valid(window));
  return window->backend->CreateVulkanSurface(vk_instance, surface_khr);
}


// WindowManagerBackend --------------------------------------------------------

std::vector<const char*>
WindowManagerBackend::GetVulkanInstanceExtensions() {
  NOT_REACHED("This function must be subclassed.");
  return {};
}

bool WindowManagerBackend::CreateVulkanSurface(void*, void*) {
  NOT_REACHED("This function must be subclassed.");
  return false;
}

// Misc ------------------------------------------------------------------------

const char* ToString(WindowBackendType type) {
  switch (type) {
    case WindowBackendType::kSDLOpenGL: return "SDLOpenGL";
    case WindowBackendType::kSDLVulkan: return "SDLVulkan";
    case WindowBackendType::kLast: return "Last";
  }

  NOT_REACHED("Unknown backend type.");
  return nullptr;
}

}  // namespace warhol
