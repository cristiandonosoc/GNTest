// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/debug/volumes.h"

#include <vector>

#include "src/camera.h"
#include "src/mesh.h"
#include "src/shader.h"
#include "src/utils/macros.h"

#include "src/utils/glm_impl.h"

namespace warhol {

namespace {

struct DebugAABB {
  Vec3 c;
  Vec3 r;
  Vec3 color;
};

struct DebugVolumesHolder {
  static DebugVolumesHolder& get() {
    static DebugVolumesHolder holder;
    return holder;
  }

  DebugVolumesHolder() {
    mesh_.Init();
    outline_shader =
        Shader::FromAssetPath("outline", "outline.vert", "outline.frag");
    assert(outline_shader.valid());
  }

  std::vector<DebugAABB> aabbs;

  void RenderAABB(const DebugAABB& aabb) {
    Vec3 r2 = aabb.r;
    r2 *= 2;
    std::vector<float> data = {
      0,    0,    0,
      r2.x, 0,    0,
      0,    0,    r2.z,
      r2.x, 0,    r2.z,
      0,    r2.y, 0,
      r2.x, r2.y, 0,
      0,    r2.y, r2.z,
      r2.x, r2.y, r2.z,
    };
    mesh_.BufferData(std::move(data),
                     {AttributeFormat({0, 3, 3 * sizeof(float), 0})});
    std::vector<uint32_t> indices = {
      0, 1,  1, 3,  3, 2,  2, 0,
      4, 5,  5, 7,  7, 6,  6, 4,
      0, 4,  1, 5,  3, 7,  2, 6,
    };
    mesh_.BufferIndices(std::move(indices));
    mesh_.Bind();

    auto base = aabb.c - aabb.r;
    outline_shader.SetMat4(
        Shader::Uniform::kModel,
        glm::translate(glm::mat4(1.0f), {base.x, base.y, base.z}));
    outline_shader.SetFloat3(Shader::Uniform::kColor, aabb.color);

    GL_CALL(glDrawElements,
            GL_LINES,
            mesh_.indices.size(),
            GL_UNSIGNED_INT,
            (void*)0);
  }

  Shader outline_shader;

 private:
  DELETE_COPY_AND_ASSIGN(DebugVolumesHolder);
  DELETE_MOVE_AND_ASSIGN(DebugVolumesHolder);

  Mesh mesh_;
};

// Render functions ------------------------------------------------------------

}  // namespace

void DebugVolumes::AABB(Vec3 center, Vec3 radius, Vec3 color) {
  DebugAABB aabb = {};
  aabb.c = center;
  aabb.r = radius;
  aabb.color = color;
  DebugVolumesHolder::get().aabbs.push_back(std::move(aabb));
}

void DebugVolumes::NewFrame() {
  DebugVolumesHolder::get().aabbs.clear();
}

void DebugVolumes::RenderVolumes(Camera* camera) {
  // AABBs
  auto& holder = DebugVolumesHolder::get();
  holder.outline_shader.Use();
  camera->SetView(&holder.outline_shader);
  camera->SetProjection(&holder.outline_shader);

  GL_CALL(glLineWidth, 3);

  for (auto& aabb : holder.aabbs)
    holder.RenderAABB(aabb);

  GL_CALL(glLineWidth, 1);
}

}  // namespace warhol
