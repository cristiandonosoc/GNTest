// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/common/renderer.h"

#include <unordered_map>

#include "warhol/utils/log.h"
#include "warhol/graphics/common/renderer_backend.h"
#include "warhol/graphics/common/mesh.h"
#include "warhol/graphics/common/shader.h"
#include "warhol/graphics/common/texture.h"
#include "warhol/window/common/window.h"

namespace warhol {

namespace {

void Reset(Renderer* renderer) {
  renderer->window = nullptr;
  renderer->backend.reset();
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
  ASSERT(it != factory_map->end())
      << "Could not find renderer backend: " << ToString(type);

  RendererBackendFactoryFunction factory = it->second;
  return factory();
}

}  // namespace

void SuscribeRendererBackendFactory(RendererType type,
                                    RendererBackendFactoryFunction factory) {
  FactoryMap* factory_map = GetFactoryMap();
  ASSERT(factory_map->find(type) == factory_map->end());
  factory_map->insert({type, factory});
}

// Renderer --------------------------------------------------------------------

Renderer::~Renderer() {
  if (Valid(this))
    ShutdownRenderer(this);
}

bool InitRenderer(Renderer* renderer, RendererType type, Window* window) {
  ASSERT(type != RendererType::kLast);

  renderer->type = type;
  renderer->backend = CreateRendererBackend(type);
  return renderer->backend->Init(renderer, window);
}

// An null backend renderer can happen if Shutdown was called before the
// destructor.
void ShutdownRenderer(Renderer* renderer) {
  ASSERT(Valid(renderer));
  renderer->backend->Shutdown();
  Reset(renderer);
}

// void WindowSizeChanged(Renderer* renderer, uint32_t width, uint32_t height) {}

// Mesh ------------------------------------------------------------------------

bool RendererStageMesh(Renderer* renderer, Mesh* mesh) {
  ASSERT(Valid(renderer));
  ASSERT(mesh->uuid.has_value());

  return renderer->backend->StageMesh(mesh);
}

void RendererUnstageMesh(Renderer* renderer, Mesh* mesh) {
  ASSERT(Valid(renderer));
  renderer->backend->UnstageMesh(mesh);
}

bool RendererIsMeshStaged(Renderer* renderer, Mesh* mesh) {
  ASSERT(Valid(renderer));
  return renderer->backend->IsMeshStaged(mesh);
}

bool RendererUploadMeshRange(Renderer* renderer, Mesh* mesh,
                             IndexRange vertex_range,
                             IndexRange index_range) {
  ASSERT(Valid(renderer));
  return renderer->backend->UploadMeshRange(mesh, vertex_range, index_range);
}

// Shader ----------------------------------------------------------------------

bool RendererParseShader(Renderer* renderer,
                         BasePaths* paths,
                         const std::string& vert_name,
                         const std::string& frag_name,
                         Shader* out) {
  ASSERT(Valid(renderer));
  return renderer->backend->ParseShader(renderer, paths, vert_name,
                                        frag_name, out);
}

bool RendererStageShader(Renderer* renderer, Shader* shader) {
  ASSERT(Valid(renderer));
  ASSERT(Valid(shader));
  ASSERT(Loaded(shader));

  return renderer->backend->StageShader(shader);
};

void RendererUnstageShader(Renderer* renderer, Shader* shader) {
  ASSERT(Valid(renderer));
  renderer->backend->UnstageShader(shader);
};

bool RendererIsShaderStaged(Renderer* renderer, Shader* shader) {
  ASSERT(Valid(renderer));
  return renderer->backend->IsShaderStaged(shader);
}

bool RendererLoadShader(Renderer* renderer, BasePaths* paths,
                        const std::string& vert_name,
                        const std::string& frag_name,
                        Shader* out) {
  if (!RendererParseShader(renderer, paths, vert_name, frag_name, out))
    return false;
  if (!RendererStageShader(renderer, out))
    return false;
  return true;
}

// Texture ---------------------------------------------------------------------

bool RendererStageTexture(Renderer* renderer, Texture* texture,
                          StageTextureConfig* config) {
  ASSERT(Valid(renderer));
  ASSERT(Valid(texture));
  ASSERT(Loaded(texture));

  return renderer->backend->StageTexture(texture, config);
}

void RendererUnstageTexture(Renderer* renderer, Texture* texture) {
  ASSERT(Valid(renderer));
  renderer->backend->UnstageTexture(texture);
}

bool RendererIsTextureStaged(Renderer* renderer, Texture* texture) {
  ASSERT(Valid(renderer));
  return renderer->backend->IsTextureStaged(texture);
}

// Frame Lifetime --------------------------------------------------------------

void RendererStartFrame(Renderer* renderer) {
  ASSERT(Valid(renderer));
  renderer->backend->StartFrame(renderer);
}

void RendererExecuteCommands(Renderer* renderer,
                             List<RenderCommand>* commands) {
  ASSERT(Valid(renderer));
  renderer->backend->ExecuteCommands(renderer, commands);
}

void RendererEndFrame(Renderer* renderer) {
  ASSERT(Valid(renderer));
  renderer->backend->EndFrame(renderer);
}

// Misc ------------------------------------------------------------------------

const char* ToString(RendererType type) {
  switch (type) {
    case RendererType::kOpenGL: return "OpenGL";
    case RendererType::kVulkan: return "Vulkan";
    case RendererType::kLast: return "Last";
  }

  NOT_REACHED() << "Invalid renderer type: " << (uint32_t)type;
  return nullptr;
}

}  // namespace warhol
