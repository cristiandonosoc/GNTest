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

// WindowManagerBackend --------------------------------------------------------

namespace {

void Move(WindowManagerBackend* from, WindowManagerBackend* to) {
  to->type = from->type;
  to->interface = std::move(from->interface);

  to->window_manager = from->window_manager;
  to->data = from->data;

  Clear(from);
}

}  // namespace

WindowManagerBackend::WindowManagerBackend() = default;
WindowManagerBackend::~WindowManagerBackend() {
  if (valid()) {
    ASSERT(data);
    interface.Shutdown(this);
  }
  Clear(this);
}

void Clear(WindowManagerBackend* backend) {
  backend->type = WindowManagerBackend::Type::kLast;
  backend->interface = {};
  backend->window_manager = nullptr;
  backend->data = nullptr;
}

WindowManagerBackend::WindowManagerBackend(WindowManagerBackend&& other) {
  Move(&other, this);
}

WindowManagerBackend&
WindowManagerBackend::operator=(WindowManagerBackend&& other) {
  if (this != &other) {
    Move(&other, this);
  }
  return *this;
}

const char*
WindowManagerBackend::TypeToString(WindowManagerBackend::Type type) {
  switch (type) {
    case WindowManagerBackend::Type::kSDLVulkan: return "SDLVulkan";
    case WindowManagerBackend::Type::kLast: return "Last";
  }

  NOT_REACHED("Unknown WindowManagerBackend::Type.");
  return nullptr;
}

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

  LOG(DEBUG) << "Getting interface for "
             << WindowManagerBackend::TypeToString(type);

  WindowManagerBackend backend;
  backend.type = type;
  backend.interface = window_template.interface;
  LOG(DEBUG) << "Before return: " << WindowManagerBackend::TypeToString(backend.type);
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
