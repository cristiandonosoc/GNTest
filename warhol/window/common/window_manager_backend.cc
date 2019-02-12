// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/window/common/window_manager_backend.h"

#include "warhol/utils/assert.h"
#include "warhol/utils/log.h"

namespace warhol {

namespace {

struct WindowManagerBackendTemplate {
  bool set = false;
  WindowManagerBackend::Interface interface = {};
};

WindowManagerBackendTemplate
gTemplates[(size_t)WindowManagerBackend::Type::kLast] = {};

}  // namespace

void SetWindowManagerBackendInterfaceTemplate(WindowManagerBackend::Type type,
    WindowManagerBackend::Interface interface) {
  size_t index = (size_t)type;
  ASSERT(index < (size_t)WindowManagerBackend::Type::kLast);
  WindowManagerBackendTemplate& window_template = gTemplates[index];
  ASSERT(!window_template.set);

  LOG(INFO) << "Setting WindowManagerBackend interface for "
            << WindowManagerBackend::TypeToString(type);

  window_template.interface = std::move(interface);
  window_template.set = true;
}

// Creates an un-initialized backend for |type|.
// That type MUST have been set for that type.
// Initialize must be called before using this.
WindowManagerBackend GetWindowManagerBackend(WindowManagerBackend::Type type) {
  size_t index = (size_t)type;
  ASSERT(index < (size_t)WindowManagerBackend::Type::kLast);
  WindowManagerBackendTemplate& window_template = gTemplates[index];
  ASSERT(window_template.set);

  WindowManagerBackend backend;
  backend.type = type;
  backend.interface = window_template.interface;
  return backend;
}

const char* WindowEvent::TypeToString(WindowEvent::Type event) {
  switch (event) {
    case WindowEvent::Type::kQuit: return "Quit";
    case WindowEvent::Type::kWindowResize: return "WindowResize";
    case WindowEvent::Type::kLast: return "Last";
  }

  NOT_REACHED("Unknown WindowEvent.");
  return nullptr;
}

}  // namespace warhol
