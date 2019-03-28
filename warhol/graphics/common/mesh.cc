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

}  // namespace

uint64_t GetNextMeshUUID() { return kNextMeshUUID++; }

size_t Hash(const Vertex& vertex) {
  return ((Hash(vertex.pos) ^ (Hash(vertex.color) << 1)) >> 1) ^
         (Hash(vertex.uv) << 1);
}

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

  mesh->vertices.reserve(attrib.vertices.size() + attrib.texcoords.size());
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
      auto [it, ok] = vertex_ht.insert({vertex_hash, mesh->vertices.size()});
      if (ok)
        mesh->vertices.push_back(std::move(vertex));
      mesh->indices.push_back(it->second);
    }
  }

  mesh->uuid = GetNextMeshUUID();
  LOG(INFO) << "Loaded model " << mesh->uuid
            << ". Vertices: " << mesh->vertices.size()
            << ", Indices: " << mesh->indices.size();
  return mesh;
}

}  // namespace warhol
