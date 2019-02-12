// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "warhol/utils/macros.h"

namespace warhol {

struct Camera;
struct Renderer;

struct RendererBackend {
  bool valid() const { return renderer != nullptr && data != nullptr; }

  // DO NOT add numbers to this enum.
  enum class Type {
    kVulkan,
    kLast,    // DO NOT specialize with numbers.
  };
  static const char* TypeToString(Type);

  RendererBackend();
  ~RendererBackend();
  DELETE_COPY_AND_ASSIGN(RendererBackend);
  DECLARE_MOVE_AND_ASSIGN(RendererBackend);

  Type type = Type::kLast;
  Renderer* renderer;   // Not-owning.

  struct Interface {
    // IMPORTANT: If you add more functions, remember to update the move ctor!
    bool (*InitFunction)(RendererBackend*) = nullptr;
    bool (*ExecuteCommands)(RendererBackend*) = nullptr;
    bool (*ShutdownFunction)(RendererBackend*) = nullptr;
    bool (*DrawFrameFunction)(RendererBackend*, Camera*) = nullptr;
  };
  Interface interface = {};

  // |data| lifecycle is managed by Init and Shutdown. Shutdown is called on the
  // destructor.
  void* data = nullptr;
};

void Clear(RendererBackend*);

// Each RendererBackend::Type will get a reference set.
void SetRendererBackendInterfaceTemplate(RendererBackend::Type,
                                         RendererBackend::Interface);

// Creates a un-initialiazed backend for |type|.
// That type MUST have a set renderer interface.
RendererBackend GetRendererBackend(RendererBackend::Type type);

}  // namespace warhol
