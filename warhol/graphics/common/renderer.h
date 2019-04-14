// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdlib.h>

#include <memory>
#include <vector>

#include "warhol/utils/macros.h"

#include "warhol/graphics/common/render_command.h"
#include "warhol/graphics/common/renderer_backend.h"

#include "warhol/math/vec.h"

namespace warhol {

struct Shader;
struct Window;

// Renderers are implemented in terms of backends (OpenGL, Vulkan, etc.).
// This means that the renderer is a simple proxy object the users are warhol
// will use, while the functionality is actually implemented by the |backend|
// object within it.

// Backend Suscription ---------------------------------------------------------

enum class RendererType {
  kOpenGL,
  kVulkan,
  kLast,
};
const char* ToString(RendererType);

// Each backend, upon application startup, must suscribe a function that will
// be called to create a that particular RendererBackend.
using RendererBackendFactoryFunction = std::unique_ptr<RendererBackend> (*)();
void SuscribeRendererBackendFactory(RendererType,
                                    RendererBackendFactoryFunction);

// Renderer --------------------------------------------------------------------

struct Renderer {
  RAII_CONSTRUCTORS(Renderer);

  Vec3 clear_color;

  Window* window = nullptr;

  RendererType type = RendererType::kLast;
  std::unique_ptr<RendererBackend> backend;
};

inline bool Valid(Renderer* r) { return !!r->backend; }

bool InitRenderer(Renderer*, RendererType, Window*);
// Will be called on destructor if the renderer is valid.
void ShutdownRenderer(Renderer*);

void WindowSizeChanged(Renderer*, uint32_t width, uint32_t height);

// Resource Uploading.

bool RendererStageMesh(Renderer*, Mesh*);
void RendererUnstageMesh(Renderer*, Mesh*);
bool RendererIsMeshStaged(Renderer*, Mesh*);

// |vertex_range| and |index_range| is what range of data of the mesh has to
// be uploaded. The format is (offset, size).
// An empty range means upload the whole used memory of each memory pool.
bool RendererUploadMeshRange(Renderer*, Mesh*,
                             IndexRange vertex_range = {},
                             IndexRange index_range = {});

bool RendererStageShader(Renderer*, Shader*);
void RendererUnstageShader(Renderer*, Shader*);
bool RendererIsShaderStaged(Renderer*, Shader*);

struct StageTextureConfig {
  enum class Wrap {
    kClampToEdge,
    kMirroredRepeat,
    kRepeat,
  };

  enum class Filter {
    kNearest,
    kLinear,
    kNearestMipmapNearest,
    kNearestMipmapLinear,
    kLinearMipmapNearest,
    kLinearMipampLinear,
  };

  bool generate_mipmaps = true;

  Wrap wrap_u = Wrap::kRepeat;
  Wrap wrap_v = Wrap::kRepeat;

  Filter min_filter = Filter::kLinear;
  Filter max_filter = Filter::kLinear;
};

bool RendererStageTexture(Renderer*, Texture*, StageTextureConfig*);
void RendererUnstageTexture(Renderer*, Texture*);
bool RendererIsTextureStaged(Renderer*, Texture*);

void RendererStartFrame(Renderer*);
void RendererExecuteCommands(Renderer*, List<RenderCommand>* commands);
void RendererEndFrame(Renderer*);

}  // namespace
