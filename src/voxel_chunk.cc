// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/voxel_chunk.h"

#include <bitset>


#include "src/graphics/GL/utils.h"
#include "src/shader.h"
#include "src/texture_atlas.h"
#include "src/utils/coords.h"
#include "src/utils/glm_impl.h"

namespace warhol {

namespace {

float indexed_vertices[] = {
  // Front.
  0.5f,  -0.5f, -0.5f,
  0.5f,  -0.5f,  0.5f,
  0.5f,   0.5f,  0.5f,
  0.5f,   0.5f, -0.5f,

  // Back.
  -0.5f, -0.5f, -0.5f,
  -0.5f, -0.5f,  0.5f,
  -0.5f,  0.5f,  0.5f,
  -0.5f,  0.5f, -0.5f,

  // Left.
  -0.5f, -0.5f, -0.5f,
   0.5f, -0.5f, -0.5f,
   0.5f,  0.5f, -0.5f,
  -0.5f,  0.5f, -0.5f,

  // Right.
  -0.5f, -0.5f, 0.5f,
   0.5f, -0.5f, 0.5f,
   0.5f,  0.5f, 0.5f,
  -0.5f,  0.5f, 0.5f,

  // Top.
  -0.5f, 0.5f, -0.5f,
   0.5f, 0.5f, -0.5f,
   0.5f, 0.5f,  0.5f,
  -0.5f, 0.5f,  0.5f,

  // Bottom.
  -0.5f, -0.5f, -0.5f,
   0.5f, -0.5f, -0.5f,
   0.5f, -0.5f,  0.5f,
  -0.5f, -0.5f,  0.5f,
};

float indexed_uvs[] = {
  0.0f, 0.0f,
  1.0f, 0.0f,
  1.0f, 1.0f,
  0.0f, 1.0f,

  0.0f, 0.0f,
  1.0f, 0.0f,
  1.0f, 1.0f,
  0.0f, 1.0f,

  0.0f, 0.0f,
  1.0f, 0.0f,
  1.0f, 1.0f,
  0.0f, 1.0f,

  0.0f, 0.0f,
  1.0f, 0.0f,
  1.0f, 1.0f,
  0.0f, 1.0f,

  0.0f, 0.0f,
  1.0f, 0.0f,
  1.0f, 1.0f,
  0.0f, 1.0f,

  0.0f, 0.0f,
  1.0f, 0.0f,
  1.0f, 1.0f,
  0.0f, 1.0f,
};

uint32_t indices[] = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4,
    8, 9, 10, 10, 11, 8,
    12, 13, 14, 14, 15, 12,
    16, 17, 18, 18, 19, 16,
    20, 21, 22, 22, 23, 20,
};

}  // namespace

Voxel::Voxel() = default;
Voxel::~Voxel() {
  if (vao_.value) {
    glDeleteVertexArrays(1, &vao_.value);
    vao_.clear();
  }
  if (vertex_vbo_.value) {
    glDeleteBuffers(1, &vertex_vbo_.value);
    vao_.clear();
  }
  if (uv_vbo1_.value) {
    glDeleteBuffers(1, &uv_vbo1_.value);
    uv_vbo1_.clear();
  }
  if (uv_vbo2_.value) {
    glDeleteBuffers(1, &uv_vbo2_.value);
    uv_vbo1_.clear();
  }
  if (ebo_.value) {
    glDeleteBuffers(1, &ebo_.value);
    ebo_.clear();
  }
}


bool Voxel::Init() {
  glGenVertexArrays(1, &vao_.value);
  glBindVertexArray(vao_.value);

  uint32_t buffers[4];
  glGenBuffers(ARRAY_SIZE(buffers), buffers);
  vertex_vbo_ = buffers[0];
  uv_vbo1_ = buffers[1];
  uv_vbo2_ = buffers[2];
  ebo_ = buffers[3];

  if (CHECK_GL_ERRORS("Creating buffers"))
    exit(1);

  uvs1_.reserve(ARRAY_SIZE(indexed_uvs));
  uvs2_.reserve(ARRAY_SIZE(indexed_uvs));
  for (size_t i = 0; i < ARRAY_SIZE(indexed_uvs); i++) {
    uvs1_.emplace_back(indexed_uvs[i]);
    uvs2_.emplace_back(indexed_uvs[i]);
  }

  // Vertices.
  glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo_.value);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(indexed_vertices),
               indexed_vertices,
               GL_STATIC_DRAW);
  // How to interpret the buffer.
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  if (CHECK_GL_ERRORS("Buffering vertices"))
    exit(1);

  // UV
  glBindBuffer(GL_ARRAY_BUFFER, uv_vbo1_.value);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(float) * uvs1_.size(),
               uvs1_.data(),
               GL_DYNAMIC_DRAW);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);

  if (CHECK_GL_ERRORS("Buffering UV1"))
    exit(1);

  // UV2
  glBindBuffer(GL_ARRAY_BUFFER, uv_vbo2_.value);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(float) * uvs2_.size(),
               uvs2_.data(),
               GL_DYNAMIC_DRAW);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(2);

  if (CHECK_GL_ERRORS("Buffering UV2"))
    exit(1);

  // Indices
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_.value);
  glBufferData(
      GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  if (CHECK_GL_ERRORS("Buffering indices"))
    exit(1);

  glBindVertexArray(NULL);
  glBindBuffer(GL_ARRAY_BUFFER, NULL);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);

  if (CHECK_GL_ERRORS("Unbinding"))
    exit(1);

  initialized_ = true;
  return true;
}

inline void
EmplaceBackCoord(std::vector<float>* mesh, float x, float y, float z) {
  mesh->emplace_back((float)x);
  mesh->emplace_back((float)y);
  mesh->emplace_back((float)z);
}

struct MeshWithIndices {
  std::vector<float> vertices;
  std::vector<uint32_t> indicies;
};

MeshWithIndices
CalculateMeshFromQuads(Pair3<size_t> o, const Quad3<size_t>& quad) {
  std::vector<float> mesh;
  mesh.reserve(3 * 4 * 4);

  Vec3 min{(float)quad.min.x, (float)quad.min.y, (float)quad.min.z};
  Vec3 max{(float)quad.max.x, (float)quad.max.y, (float)quad.max.z};

  max.z += 0.75f;


  // Indexed

  EmplaceBackCoord(&mesh, o.x + min.x, o.y + min.y, o.z + min.z);
  EmplaceBackCoord(&mesh, o.x + min.x, o.y + min.y, o.z + max.z);
  EmplaceBackCoord(&mesh, o.x + min.x, o.y + max.y, o.z + min.z);
  EmplaceBackCoord(&mesh, o.x + min.x, o.y + max.y, o.z + max.z);

  EmplaceBackCoord(&mesh, o.x + min.x, o.y + min.y, o.z + max.z);
  EmplaceBackCoord(&mesh, o.x + max.x, o.y + min.y, o.z + max.z);
  EmplaceBackCoord(&mesh, o.x + min.x, o.y + max.y, o.z + max.z);
  EmplaceBackCoord(&mesh, o.x + max.x, o.y + max.y, o.z + max.z);

  EmplaceBackCoord(&mesh, o.x + max.x, o.y + min.y, o.z + max.z);
  EmplaceBackCoord(&mesh, o.x + max.x, o.y + min.y, o.z + min.z);
  EmplaceBackCoord(&mesh, o.x + max.x, o.y + max.y, o.z + max.z);
  EmplaceBackCoord(&mesh, o.x + max.x, o.y + max.y, o.z + min.z);

  EmplaceBackCoord(&mesh, o.x + max.x, o.y + min.y, o.z + min.z);
  EmplaceBackCoord(&mesh, o.x + min.x, o.y + min.y, o.z + min.z);
  EmplaceBackCoord(&mesh, o.x + max.x, o.y + max.y, o.z + min.z);
  EmplaceBackCoord(&mesh, o.x + min.x, o.y + max.y, o.z + min.z);

  std::vector<uint32_t> indices = {
      0, 1, 2,  2,  3,  0, 4,  5,  6,  6,  7,  4,
      8, 9, 10, 10, 11, 8, 12, 13, 14, 14, 15, 12,
      /* 16, 17, 18, 18, 19, 16, */
      /* 20, 21, 22, 22, 23, 20, */
  };

  // TODO(Cristian): botton and top faces.
  return { std::move(mesh), std::move(indices) };
}

bool VoxelChunk::InitializedGreedy() {

  elements_[2].type = VoxelElement::Type::kNone;


  glGenVertexArrays(1, &vao_.value);
  glBindVertexArray(vao_.value);

  uint32_t buffers[2];
  glGenBuffers(ARRAY_SIZE(buffers), buffers);
  vbo_ = buffers[0];
  /* uv_vbo1_ = buffers[1]; */
  /* uv_vbo2_ = buffers[2]; */
  ebo_ = buffers[1];

  if (CHECK_GL_ERRORS("Creating buffers"))
    exit(1);

  /* uvs1_.reserve(ARRAY_SIZE(indexed_uvs)); */
  /* uvs2_.reserve(ARRAY_SIZE(indexed_uvs)); */
  /* for (size_t i = 0; i < ARRAY_SIZE(indexed_uvs); i++) { */
  /*   uvs1_.emplace_back(indexed_uvs[i]); */
  /*   uvs2_.emplace_back(indexed_uvs[i]); */
  /* } */


  // Vertices.
  glBindBuffer(GL_ARRAY_BUFFER, vbo_.value);
  /* glBufferData(GL_ARRAY_BUFFER, */
  /*              sizeof(indexed_vertices), */
  /*              indexed_vertices, */
  /*              GL_STATIC_DRAW); */
  // How to interpret the buffer.
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  if (CHECK_GL_ERRORS("Buffering vertices"))
    exit(1);

  /* // UV */
  /* glBindBuffer(GL_ARRAY_BUFFER, uv_vbo1_.value); */
  /* glBufferData(GL_ARRAY_BUFFER, */
  /*              sizeof(float) * uvs1_.size(), */
  /*              uvs1_.data(), */
  /*              GL_DYNAMIC_DRAW); */
  /* glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0); */
  /* glEnableVertexAttribArray(1); */

  /* if (CHECK_GL_ERRORS("Buffering UV1")) */
  /*   exit(1); */

  /* // UV2 */
  /* glBindBuffer(GL_ARRAY_BUFFER, uv_vbo2_.value); */
  /* glBufferData(GL_ARRAY_BUFFER, */
  /*              sizeof(float) * uvs2_.size(), */
  /*              uvs2_.data(), */
  /*              GL_DYNAMIC_DRAW); */
  /* glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0); */
  /* glEnableVertexAttribArray(2); */

  /* if (CHECK_GL_ERRORS("Buffering UV2")) */
  /*   exit(1); */

  // Indices
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_.value);
  glBufferData(
      GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  if (CHECK_GL_ERRORS("Buffering indices"))
    exit(1);

  glBindVertexArray(NULL);
  glBindBuffer(GL_ARRAY_BUFFER, NULL);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);

  if (CHECK_GL_ERRORS("Unbinding"))
    exit(1);

  quads_ = GreedyMesh();

  greedy = true;
  return true;
}

#include <sstream>

void PrintUVs(const std::vector<float>& uvs) {
  std::stringstream ss;
  ss << "UVS: " << std::endl;
  for (size_t i = 0; i < 6; i++) {
    size_t offset = 8 * i;
    ss
    << uvs[offset + 0] << ", " << uvs[offset + 1] << std::endl
    << uvs[offset + 2] << ", " << uvs[offset + 3] << std::endl
    << uvs[offset + 4] << ", " << uvs[offset + 5] << std::endl
    << uvs[offset + 6] << ", " << uvs[offset + 7] << std::endl
    << "---------------------------------------" << std::endl;
  }
  LOG(DEBUG) << ss.str();
}

VoxelElement& VoxelChunk::GetVoxelElement(size_t x, size_t y, size_t z) {
  size_t index = Coord3ToArrayIndex(kVoxelChunkSize, x, y, z);
  return elements_[index];
}

namespace {

void
ChangeUV(Voxel::Face face,
         Pair<Pair<float>> min_max_uvs,
         int vbo,
         std::vector<float>* uvs) {
  /* auto uv_coords = atlas.GetUVs(index); */
  uint32_t offset =
      2 * 4 * (uint32_t)face - (uint32_t)Voxel::Face::kFront;
  uvs->at(offset + 0) = min_max_uvs.x.x;
  uvs->at(offset + 1) = min_max_uvs.x.y;
  uvs->at(offset + 2) = min_max_uvs.y.x;
  uvs->at(offset + 3) = min_max_uvs.x.y;
  uvs->at(offset + 4) = min_max_uvs.y.x;
  uvs->at(offset + 5) = min_max_uvs.y.y;
  uvs->at(offset + 6) = min_max_uvs.x.x;
  uvs->at(offset + 7) = min_max_uvs.y.y;

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferSubData(GL_ARRAY_BUFFER,
                  offset * sizeof(float),
                  8 * sizeof(float),
                  uvs->data() + offset);
  glBindBuffer(GL_ARRAY_BUFFER, NULL);
}

}  // namespace

std::vector<std::vector<Quad3<size_t>>>
VoxelChunk::GreedyMesh() {
  // We iterate over z and creating the greatest chunks we can.
  std::vector<std::vector<Quad3<size_t>>> quads3d;
  constexpr size_t side = kVoxelChunkSize;
  for (size_t z = 0; z < side; z++) {

    // Look over which voxels are there.
    std::bitset<side * side> mask;
    for (size_t y = 0; y < side; y++) {
      for (size_t x = 0; x < side; x++) {
        size_t index = Coord3ToArrayIndex(side, x, y, z);
        mask[index] = (bool)elements_[index];
        LOG(DEBUG) << "Set mask at [" << Pair3<size_t>{x, y, z}.ToString() << "] to: " << mask[index];
      }
    }


    LOG(DEBUG) << "------------------------------\n\n";


    LOG(DEBUG) <<  "Checking layer z: " << z;



    // Now that we have a mask, we can start greedily meshing.
    // TODO(Cristian): Can we update the mask on the fly and not dot 2 passes?
    std::vector<Quad3<size_t>> quads;
    for (size_t y = 0; y < side; y++) {
      for (size_t x = 0; x < side; x++) {

        const char* indent = "  ";
        size_t index = Coord3ToArrayIndex(side, x, y, z);
        // We found a quad, we see how big of a grouping we can do.
        if (mask[index]) {
          Quad3<size_t> quad = {};
          quad.min = {x, y, z};
          quad.max = {x, y, z};

          LOG(DEBUG) << indent << "Found free block at " << quad.min.ToString();
          indent = "    ";


          // We look over the X-axis to see how big this chunk is.
          for (size_t ix = x + 1; ix < side; ix++) {
            size_t new_index = Coord3ToArrayIndex(side, ix, y, z);
            if (!mask[new_index]) {
              LOG(DEBUG) << indent << "Did not find free X at: " << Pair3<size_t>{ix, y, z}.ToString();
              break;
            }

            quad.max.x = ix;
            mask[new_index] = false;
          }

          LOG(DEBUG) << indent << "Extended X to: " << quad.max.ToString();

          // We go over Y row for row to see if this chunk extends
          bool found = true;
          for (size_t iy = y + 1; iy < side; iy++) {
            // If any in this row doesn't match, this quad is not extensible.
            for (size_t ix = x; ix <= quad.max.x; ix++) {
              size_t new_index = Coord3ToArrayIndex(side, ix, iy, z);
              if (!mask[new_index]) {
                found = false;
                break;
              }
            }

            // We check if the row did extend the piece.
            // If not, we could not extend and don't mark anything.
            if (!found) {
              LOG(DEBUG) << indent << "Did not find a Y extension";
              break;
            }


            // We were able to extend the chunk, so we mark it as not available
            // anymore.
            quad.max.y = iy;

            LOG(DEBUG) << indent << "Found and Y extension to: " << quad.max.ToString();

            for (size_t ix = x; ix <= quad.max.x; ix++) {
              size_t new_index = Coord3ToArrayIndex(side, ix, iy, z);
              mask[new_index] = false;
            }
          }

          // Now we have the quad as big as we could extend it, first X-wise and
          // then Y-wise, so we add it to the arrays.
          quads.push_back(std::move(quad));
        }
      }
    }

    // We finished with this layer.
    quads3d.push_back(std::move(quads));
  }

  LOG(DEBUG) << "----------------------\n\n\n";

  return quads3d;
}

// VoxelChunk ------------------------------------------------------------------


void
ChangeUV(const TextureAtlas& atlas,
         Voxel* cube,
         Voxel::Face face,
         int texture_index,
         int layer) {
  auto uv_coords = atlas.GetUVs(texture_index);
  cube->SetFace(face, layer, uv_coords);
}



void SetCubeFace(const TextureAtlas& atlas,
                 Voxel* cube,
                 Voxel::Face face,
                 int texture_index1, int texture_index2) {
  ChangeUV(atlas, cube, face, texture_index1, 0);
  ChangeUV(atlas, cube, face, texture_index2, 1);
}



VoxelChunk::VoxelChunk() = default;
VoxelChunk::VoxelChunk(TextureAtlas* atlas) : atlas_(atlas) {}

Voxel& VoxelChunk::GetVoxel(size_t x, size_t y, size_t z) {
  assert(x < kVoxelChunkSize && y < kVoxelChunkSize && z < kVoxelChunkSize);
  size_t z_offset = z * kVoxelChunkSize * kVoxelChunkSize;
  size_t y_offset = y * kVoxelChunkSize;
  return voxels_[z_offset + y_offset + x];
}

bool VoxelChunk::Init() {
  for (size_t x = 0; x < kVoxelChunkSize; x++) {
    for (size_t y = 0; y < kVoxelChunkSize; y++) {
      for (size_t z = 0; z < kVoxelChunkSize; z++) {
        Voxel& voxel = GetVoxel(x, y, z);
        if (!voxel.Init())
          return false;

        // TODO(Cristian): Offset this by the chunk position offset.
        // TODO(Cristian): Later do a scene graph.
        voxel.set_position({x, y, z});
        SetCubeFace(*atlas_, &voxel, Voxel::Face::kBack,
                    kGrassDirt, kTransparent);
        SetCubeFace(*atlas_, &voxel, Voxel::Face::kFront,
                    kGrassDirt, kTransparent);
        SetCubeFace(*atlas_, &voxel, Voxel::Face::kLeft,
                    kGrassDirt, kCrack4);
        SetCubeFace(*atlas_, &voxel, Voxel::Face::kRight,
                    kGrassDirt, kTransparent);
        SetCubeFace(*atlas_, &voxel, Voxel::Face::kTop,
                    kGrass, kCrack9);
        SetCubeFace(*atlas_, &voxel, Voxel::Face::kBottom,
                    kDirt, kTransparent);
      }
    }
  }
  return true;
}

void
VoxelChunk::Render(Shader* shader) {
  if (!greedy) {
    for (Voxel& voxel : voxels_) voxel.Render(shader);
  } else {

    glBindVertexArray(vao_.value);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_.value);

    // TODO(donosoc): Do this only when needed.
    shader->SetMat4("model", glm::mat4(1.0f));

    for (auto& z_quads : quads_) {
      for (Quad3<size_t>& quad : z_quads) {
        auto meshi = CalculateMeshFromQuads({5, 0, 5}, quad);

        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(float) * meshi.vertices.size(),
                     meshi.vertices.data(),
                     GL_DYNAMIC_DRAW);

        glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_INT, NULL);
      }
    }
  }
}

void
Voxel::SetFace(Voxel::Face face,
                       int layer,
                       Pair<Pair<float>> min_max_uvs) {
  if (layer == 0) {
    ChangeUV(face, std::move(min_max_uvs), uv_vbo1_.value, &uvs1_);
  } else if (layer == 1) {
    ChangeUV(face, std::move(min_max_uvs), uv_vbo2_.value, &uvs2_);
  } else {
    LOG(ERROR) << "Wrong layer count: " << layer;
    exit(1);
  }
}

void Voxel::Render(Shader* shader) {
  assert(initialized_);
  glBindVertexArray(vao_.value);
  // TODO(donosoc): Do this only when needed.
  model_ = glm::translate(glm::mat4(1.0f), position_);
  shader->SetMat4("model", model_);
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, NULL);
}

}  // namespace warhol
