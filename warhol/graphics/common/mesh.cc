// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/common/mesh.h"

#include <atomic>
#include <unordered_map>

#include <third_party/tiny_obj_loader/tiny_obj_loader.h>

#include "warhol/utils/assert.h"
#include "warhol/utils/log.h"

namespace warhol {

namespace {

std::atomic<uint64_t> kNextMeshUUID = 1;

size_t Hash(const Vertex& vertex) {
  return ((Hash(vertex.pos) ^ (Hash(vertex.color) << 1)) >> 1) ^
         (Hash(vertex.uv) << 1);
}

}  // namespace

Mesh::~Mesh() {
  if (Staged(this))
    NOT_REACHED("Mesh has not been unstaged.");
}

uint64_t GetNextMeshUUID() { return kNextMeshUUID++; }


bool LoadMesh(const std::string& model_path, Mesh* mesh) {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn, err;
  if (!tinyobj::LoadObj(
          &attrib, &shapes, &materials, &warn, &err, model_path.data())) {
    LOG(ERROR) << "Could not load model in " << model_path.data();
    return false;
  }

  // We only want to insert unique vertices.
  std::unordered_map<size_t, uint32_t> vertex_ht;

  // TODO: Precalculate the size needed instead of copying everything over...
  //       twice!

  // We reserve a huuuuge memory pool for the vertices and indices.
  MemoryPool vert_pool;
  InitMemoryPool(&vert_pool, MEGABYTES(64));
  size_t vert_count = 0;

  MemoryPool index_pool;
  InitMemoryPool(&index_pool, MEGABYTES(16));
  size_t index_count = 0;

  for (const auto& shape : shapes) {
    for (const auto& index : shape.mesh.indices) {
      Vertex vertex = {};
      vertex.pos[0] = attrib.vertices[3 * index.vertex_index + 0];
      vertex.pos[1] = attrib.vertices[3 * index.vertex_index + 1];
      vertex.pos[2] = attrib.vertices[3 * index.vertex_index + 2];

      vertex.color = {1.0f, 1.0f, 1.0f};

      vertex.uv[0] = attrib.texcoords[2 * index.texcoord_index + 0];
      // OBJ assumes bottom-left corner as 0.
      vertex.uv[1] = 1.0f - attrib.texcoords[2 * index.texcoord_index + 1];

      size_t vertex_hash = Hash(vertex);
      // We see if we need to emplace it.
      auto [it, ok] = vertex_ht.insert({vertex_hash, Used(&vert_pool)});
      if (ok) {
        Push(&vert_pool, std::move(vertex));
        vert_count++;
      }
      Push(&index_pool, (uint32_t)it->second);
      index_count++;
    }
  }

  // Now that we have the loaded sizes, we can move it over.
  // TODO: Pools should have some kind of bucketing instead of giving you the
  //       exact amount.
  InitMeshPools(mesh, Used(&vert_pool), Used(&index_pool));

  memcpy(mesh->vertices.data.get(), vert_pool.data.get(), Used(&vert_pool));
  mesh->vertex_size = sizeof(Vertex);
  mesh->vertex_count = vert_count;

  memcpy(mesh->indices.data.get(), index_pool.data.get(), Used(&index_pool));
  mesh->index_count = index_count;

  mesh->uuid = GetNextMeshUUID();
  LOG(INFO) << "Loaded model " << mesh->uuid.value << std::endl
            << "Vertices: " << mesh->vertex_count << "("
            << BytesToString(VerticesSize(mesh)) << ")" << std::endl
            << "Indices: " << mesh->index_count << "("
            << BytesToString(IndicesSize(mesh));
  return mesh;
}

void InitMeshPools(Mesh* mesh, size_t vert_size, size_t index_size) {
  LOG(DEBUG) << "Initializing mesh pools. Vertex: " << vert_size << " ("
             << BytesToString(vert_size) << ")"
             << ", indices: " << index_size << " (" << BytesToString(index_size)
             << ").";
  ASSERT(!Valid(&mesh->vertices));
  mesh->vertices.name = "Vertex";
  InitMemoryPool(&mesh->vertices, vert_size);
  ASSERT(!Valid(&mesh->indices));
  mesh->indices.name = "Indices";
  InitMemoryPool(&mesh->indices, index_size);
}

uint32_t AttributesSize(Mesh* mesh) {
  uint32_t total = 0;
  for (auto & attribute : mesh->attributes) {
    ASSERT(attribute.type != AttributeType::kLast);
    total += (uint32_t)attribute.type * attribute.count;
  }

  return total;
}

void ResetMesh(Mesh* mesh) {
  mesh->vertex_count = 0;
  mesh->index_count = 0;
  ResetMemoryPool(&mesh->vertices);
  ResetMemoryPool(&mesh->indices);
}

}  // namespace warhol
