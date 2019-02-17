// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "warhol/utils/macros.h"

namespace warhol {

struct Camera;
struct Renderer;

struct RendererBackend {
  bool valid() const;

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
    bool (*Init)(RendererBackend*) = nullptr;
    void(*Shutdown)(RendererBackend*) = nullptr;
    bool (*ExecuteCommands)(RendererBackend*) = nullptr;
    bool (*DrawFrame)(RendererBackend*, Camera*) = nullptr;
  };
  Interface interface = {};

  // |data| lifecycle is managed by Init and Shutdown. Shutdown is called on the
  // destructor.
  void* data = nullptr;
};

// Each RendererBackend::Type will get a reference set.
void SetRendererBackendInterfaceTemplate(RendererBackend::Type,
                                         RendererBackend::Interface);

// Creates a un-initialiazed backend for |type|.
// That type MUST have a set renderer interface.
RendererBackend GetRendererBackend(RendererBackend::Type type);

// Cleans up all the fields of a RendererBackend.
// Safe to call from a destructor.
void Clear(RendererBackend*);

}  // namespace warhol
