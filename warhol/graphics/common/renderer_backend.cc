// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/common/renderer_backend.h"

#include "warhol/utils/assert.h"
#include "warhol/utils/log.h"

namespace warhol {

namespace {

// RendererBackend Setup Interface ---------------------------------------------

// This will have one entry per backend type.
struct BackendTemplate {
  bool set = false;
  RendererBackend::Interface interface = {};
};

BackendTemplate gBackends[(size_t)RendererBackend::Type::kLast] = {};

}  // namespace

void
SetRendererBackendInterfaceTemplate(RendererBackend::Type type,
                                    RendererBackend::Interface interface) {
  size_t index = (size_t)type;
  ASSERT(index < (size_t)RendererBackend::Type::kLast);
  BackendTemplate& backend_template = gBackends[index];
  ASSERT(!backend_template.set);

  LOG(INFO) << "Setting up rendering interface for "
            << RendererBackend::TypeToString(type);

  backend_template.interface = std::move(interface);
  backend_template.set = true;
}

RendererBackend GetRendererBackend(RendererBackend::Type type) {
  size_t index = (size_t)type;
  ASSERT(index < (size_t)RendererBackend::Type::kLast);
  BackendTemplate& backend_template = gBackends[index];
  ASSERT(backend_template.set);

  RendererBackend backend;
  backend.type = type;
  backend.interface = backend_template.interface;
  return backend;
}


// RendererBackend -------------------------------------------------------------

namespace {

void Move(RendererBackend* from, RendererBackend* to) {
  to->type = from->type;
  to->renderer = from->renderer;

  to->interface = std::move(from->interface);
  to->data = from->data;

  Clear(from);
}

}  // namespace

RendererBackend::RendererBackend() = default;
RendererBackend::~RendererBackend() {
  if (valid()) {
    ASSERT(data);
    interface.Shutdown(this);   // Frees data.
  }
  Clear(this);
}

RendererBackend::RendererBackend(RendererBackend&& other) {
  Move(&other, this);
}

RendererBackend& RendererBackend::operator=(RendererBackend&& other) {
  if (this != &other)
    Move(&other, this);
  return *this;
}

void Clear(RendererBackend* backend) {
  backend->type = RendererBackend::Type::kLast;
  backend->renderer = nullptr;

  backend->interface = {};
  backend->data = nullptr;
}

bool RendererBackend::valid() const {
  if (type == Type::kLast || renderer == nullptr || data == nullptr)
    return false;
  return true;
}

const char* RendererBackend::TypeToString(RendererBackend::Type type) {
  switch (type) {
    case RendererBackend::Type::kVulkan: return "Vulkan";
    case RendererBackend::Type::kLast: return "Last";
  }

  NOT_REACHED("Unknown RendererBackendType.");
  return nullptr;
}

}  // namespace warhol
