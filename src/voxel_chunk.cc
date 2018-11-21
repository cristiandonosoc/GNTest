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

inline void
EmplaceBackUV(std::vector<float>* mesh, float u, float v) {
  mesh->emplace_back(u);
  mesh->emplace_back(v);
}

struct MeshWithIndices {
  std::vector<float> vertices;
  std::vector<float> uvs;
  std::vector<uint32_t> indices;
};

MeshWithIndices
CalculateMeshFromFace(const TextureAtlas& atlas, const VoxelTypeQuad& face) {
  MeshWithIndices mesh;
  /* constexpr size_t faces_count = 4; */
  /* mesh.reserve(3 * 4 * faces_count + 2 * 4 * faces_count); */

  auto& face_min = face.quad.min;
  auto& face_max = face.quad.max;
  Vec3 min{(float)face_min.x, (float)face_min.y, (float)face_min.z};
  Vec3 max{(float)face_max.x, (float)face_max.y, (float)face_max.z};

  max += {1.0f, 1.0f, 1.0f};

  auto uvs = atlas.GetUVs(static_cast<uint8_t>(face.type));

  EmplaceBackCoord(&mesh.vertices, min.x, min.y, min.z);
  EmplaceBackCoord(&mesh.vertices, min.x, min.y, max.z);
  EmplaceBackCoord(&mesh.vertices, max.x, max.y, max.z);
  EmplaceBackCoord(&mesh.vertices, min.x, max.y, max.z);

  EmplaceBackUV(&mesh.uvs, uvs.min().x, uvs.min().y);
  EmplaceBackUV(&mesh.uvs, uvs.min().x, uvs.max().y);
  EmplaceBackUV(&mesh.uvs, uvs.max().x, uvs.min().y);
  EmplaceBackUV(&mesh.uvs, uvs.max().x, uvs.max().y);

  /* EmplaceBackUV(&mesh, uvs.min().x, uvs.min().y); */
  /* EmplaceBackUV(&mesh, uvs.min().x, uvs.max().y + max.z); */
  /* EmplaceBackUV(&mesh, uvs.max().x + max.y, uvs.min().y); */
  /* EmplaceBackUV(&mesh, uvs.max().x + max.y, uvs.max().y + max.z); */



  return mesh;

  /* // Indexed */
  /* // X constant. */
  /* EmplaceBackCoord(&mesh, min.x, min.y, min.z); */
  /* EmplaceBackCoord(&mesh, min.x, min.y, max.z); */
  /* EmplaceBackCoord(&mesh, min.x, max.y, min.z); */
  /* EmplaceBackCoord(&mesh, min.x, max.y, max.z); */
  /* // Z constant. */
  /* EmplaceBackCoord(&mesh, min.x, min.y, max.z); */
  /* EmplaceBackCoord(&mesh, max.x, min.y, max.z); */
  /* EmplaceBackCoord(&mesh, min.x, max.y, max.z); */
  /* EmplaceBackCoord(&mesh, max.x, max.y, max.z); */

  /* // X constant. */
  /* EmplaceBackCoord(&mesh, max.x, min.y, max.z); */
  /* EmplaceBackCoord(&mesh, max.x, min.y, min.z); */
  /* EmplaceBackCoord(&mesh, max.x, max.y, max.z); */
  /* EmplaceBackCoord(&mesh, max.x, max.y, min.z); */

  /* // Y constant. */
  /* EmplaceBackCoord(&mesh, max.x, min.y, min.z); */
  /* EmplaceBackCoord(&mesh, min.x, min.y, min.z); */
  /* EmplaceBackCoord(&mesh, max.x, max.y, min.z); */
  /* EmplaceBackCoord(&mesh, min.x, max.y, min.z); */

  /* EmplaceBackUV(&mesh, uvs.min().x, uvs.min().y); */
  /* EmplaceBackUV(&mesh, uvs.min().x, uvs.max().y + max.z); */
  /* EmplaceBackUV(&mesh, uvs.max().x + max.y, uvs.min().y); */
  /* EmplaceBackUV(&mesh, uvs.max().x + max.y, uvs.max().y + max.z); */

  /* EmplaceBackUV(&mesh, uvs.min().x, uvs.min().y); */
  /* EmplaceBackUV(&mesh, uvs.max().x + max.x, uvs.min().y); */
  /* EmplaceBackUV(&mesh, uvs.min().x, uvs.max().y + max.y); */
  /* EmplaceBackUV(&mesh, uvs.max().x, uvs.max().y); */

  /* EmplaceBackUV(&mesh, uvs.min().x, uvs.max().y); */
  /* EmplaceBackUV(&mesh, uvs.min().x, uvs.min().y); */
  /* EmplaceBackUV(&mesh, uvs.max().x, uvs.max().y); */
  /* EmplaceBackUV(&mesh, uvs.max().x, uvs.min().y); */

  /* EmplaceBackUV(&mesh, uvs.max().x, uvs.min().y); */
  /* EmplaceBackUV(&mesh, uvs.min().x, uvs.min().y); */
  /* EmplaceBackUV(&mesh, uvs.max().x, uvs.max().y); */
  /* EmplaceBackUV(&mesh, uvs.min().x, uvs.max().y); */



  /* std::vector<uint32_t> indices = { */
  /*      0,  1,  2,  2,  1,  3, */
  /*      4,  5,  6,  6,  5,  7, */
  /*      8,  9, 10, 10,  9, 11, */
  /*     12, 13, 14, 14, 13, 15, */
  /*     // TODO(Cristian): botton and top faces. */
  /* }; */

  return { std::move(mesh), std::move(indices) };
}

bool VoxelChunk::InitialiazeGreedy() {
  size_t index = Coord3ToArrayIndex(kVoxelChunkSize, 1, 1, 0);
  elements_[index].type = VoxelElement::Type::kNone;

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
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  if (CHECK_GL_ERRORS("Buffering vertices"))
    exit(1);

  // UV
  /* glBindBuffer(GL_ARRAY_BUFFER, uv_vbo1_.value); */
  /* glBufferData(GL_ARRAY_BUFFER, */
  /*              sizeof(float) * uvs1_.size(), */
  /*              uvs1_.data(), */
  /*              GL_DYNAMIC_DRAW); */
  glVertexAttribPointer(
      1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

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

static VoxelElement invalid_element = {
  .type = VoxelElement::Type::kNone
};

VoxelElement& VoxelChunk::operator[](int index) {
  if (index < 0 || (size_t)index >= ARRAY_SIZE(elements_))
    return invalid_element;
  return elements_[index];
}

VoxelElement& VoxelChunk::GetVoxelElement(int x, int y, int z) {
  if (x < 0 || y < 0 || z < 0)
    return invalid_element;
  size_t index = Coord3ToArrayIndex(kVoxelChunkSize, x, y, z);
  if (index >= ARRAY_SIZE(elements_))
    return invalid_element;
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



#include "src/sdl2/def.h"

std::vector<TypedFace> VoxelChunk::CalculateFaces() {
  // We iterate over z and creating the greatest chunks we can.
  std::vector<ExpandedVoxel> expanded_voxels = ExpandVoxels();

  // Get the faces from the expanded voxels.
  std::vector<TypedFace> faces;

  for (auto& expanded_voxel : expanded_voxels) {
    // X faces.
    auto& quad = expanded_voxel.quad;
    auto x_faces_down =
        CalculateFacesX(expanded_voxel.type, quad.min.x - 1, quad.min.x, quad);
    faces.insert(faces.end(), x_faces_down.begin(), x_faces_down.end());
    auto x_faces_up =
        CalculateFacesX(expanded_voxel.type, quad.max.x + 1, quad.max.x, quad);
    faces.insert(faces.end(), x_faces_up.begin(), x_faces_up.end());
  }

  return faces;
}

std::vector<ExpandedVoxel> VoxelChunk::ExpandVoxels() {
  constexpr int side = kVoxelChunkSize;

  // Look over which voxels are there and create one big mask 3D matrix.
  std::bitset<side * side * side> mask;
  for (int y = 0; y < side; y++) {
    for (int z = 0; z < side; z++) {
      for (int x = 0; x < side; x++) {
        int index = Coord3ToArrayIndex(side, x, y, z);
        mask[index] = (bool)elements_[index];
      }
    }
  }

  std::vector<ExpandedVoxel> expanded_voxels;

  // Iterate from bottom to top (in our view, that's the Y axis).
  for (int y = 0; y < side; y++) {
    LOG(DEBUG) << "------------------------------";
    LOG(DEBUG) <<  "Checking Y layer " << y;

    // Now that we have a mask, we can start greedily meshing.
    // TODO(Cristian): Can we update the mask on the fly and not dot 2 passes?

    /* std::vector<VoxelTypeQuad> faces; */
    for (int z = 0; z < side; z++) {
      for (int x = 0; x < side; x++) {

        const char* indent = "  ";
        int index = Coord3ToArrayIndex(side, x, y, z);
        // We found a quad, we see how big of a grouping we can do.
        if (mask[index]) {
          Quad3<int> quad = {};
          quad.min = {x, y, z};
          quad.max = {x, y, z};

          LOG(DEBUG) << indent << "Found free block at " << quad.min.ToString();
          indent = "    ";

          LOG(DEBUG) << "Looking for X extension";
          indent = "      ";

          // We look over the X-axis to see how big this chunk is.
          for (int ix = x + 1; ix < side; ix++) {
            int new_index = Coord3ToArrayIndex(side, ix, y, z);
            if (!mask[new_index]) {
              LOG(DEBUG) << indent << "Did not find free X at: "
                         << Pair3<int>{ix, y, z}.ToString();
              break;
            }

            quad.max.x = ix;
            mask[new_index] = false;
          }

          LOG(DEBUG) << indent
                     << "Extended X to: " << quad.max.ToString();

          indent = "    ";
          LOG(DEBUG) << indent << "Looking for Z extension";
          indent = "      ";

          // We go over Z row for row to see if this chunk extends
          bool found_z_extension = true;
          for (int iz = z + 1; iz < side; iz++) {
            // If any in this row doesn't match, this quad is not extensible.
            for (int ix = x; ix <= quad.max.x; ix++) {
              int new_index = Coord3ToArrayIndex(side, ix, y, iz);
              if (!mask[new_index]) {
                found_z_extension = false;
                break;
              }
            }

            // We check if the row did extend the piece.
            // If not, we could not extend and don't mark anything.
            if (!found_z_extension) {
              LOG(DEBUG) << indent << "Did not find a Z extension";
              break;
            }

            // We were able to extend the chunk, so we mark it as not available
            // anymore.
            quad.max.z = iz;

            LOG(DEBUG) << indent
                       << "Found and Z extension to: " << quad.max.ToString();

            for (int ix = x; ix <= quad.max.x; ix++) {
              int new_index = Coord3ToArrayIndex(side, ix, y, iz);
              mask[new_index] = false;
            }
          }

          if (found_z_extension) {
            LOG(DEBUG) << indent
                       << "Extended Z to: " << quad.max.ToString();
          }

          indent = "    ";
          LOG(DEBUG) << indent << "Looking for Y extension";
          indent = "      ";

          // Now we see if we can make it grow upwards.
          bool found_y_extension = true;
          for (int iy = y + 1; iy < side; iy++) {
            for (int iz = quad.min.z; iz <= quad.max.z; iz++) {
              for (int ix = quad.min.x; ix <= quad.max.x; ix++) {
                // We see if in this current x row we could extend all the way.
                int new_index = Coord3ToArrayIndex(side, ix, iy, iz);
                if (!mask[new_index]) {
                  LOG(DEBUG)
                      << indent
                      << "FAILED AT: " << Pair3<int>{ix, iy, iz}.ToString()
                      << " (INDEX: " << new_index
                      << ") (MASK: " << mask[new_index] << ")";
                  found_y_extension = false;
                  break;
                }
              }

              // We iterated over the current X row for this Z value and we
              // couldn't find the extension.
              if (!found_y_extension)
                break;
            }

            // At this point, we iterated over the whole "plane" at this Y level
            // and if we didn't find an extension, simply ignore it.
            if (!found_y_extension) {
              LOG(DEBUG) << indent << "Did not find Y extension";
              break;
            }

            LOG(DEBUG) << indent << "Found Y extension from " << y << " to "
                       << iy;

            // We found an extension upwards! We need to also mark whole plane
            // as found (a lot of iteration :| ).
            quad.max.y = iy;
            for (int iz = quad.min.z; iz <= quad.max.z; iz++) {
              for (int ix = quad.min.x; ix <= quad.max.x; ix++) {
                int new_index = Coord3ToArrayIndex(side, ix, iy, iz);
                mask[new_index] = false;
              }
            }
          }

          if (found_y_extension) {
            LOG(DEBUG) << indent << "Extended Y to: " << quad.max.ToString();
          }


          ExpandedVoxel expanded_voxel;
          expanded_voxel.quad = std::move(quad);
          expanded_voxel.type = VoxelType::kDirt;
          expanded_voxels.push_back(std::move(expanded_voxel));

#if 0
          // Now that we have the quad as big as it gets, we need to know how
          // much of the face is visible. We do this by repeating the greedy
          // algorithm, but per face.
          std::vector<Quad3<int>> faces;

          // X faces.
          auto x_faces_down = CalculateFacesX(quad.min.x - 1, quad.min.x, quad);
          faces.insert(faces.end(), x_faces_down.begin(), x_faces_down.end());
          auto x_faces_up = CalculateFacesX(quad.max.x + 1, quad.max.x, quad);
          faces.insert(faces.end(), x_faces_up.begin(), x_faces_up.end());

          // Now we have the quad as big as we could extend it, first X-wise and
          // then Z-wise, so we add it to the arrays.

          for (auto& face : faces) {
            VoxelTypeQuad typed_quad;
            typed_quad.type = VoxelElement::Type::kDirt;
            typed_quad.quad = std::move(face);
            quads.push_back(std::move(typed_quad));
          }
        }
#endif
      }
    }
  }

  return expanded_voxels;
}

std::vector<Quad3<int>>
VoxelChunk::CalculateFacesX(int x, int x_to_check, Quad3<int> quad) {
  std::vector<Quad3<int>> faces;
  constexpr int side = kVoxelChunkSize;

  std::bitset<side * side> slots;
  for (int y = quad.min.y; y <= quad.max.y; y++) {
    for (int z = quad.min.y; z <= quad.max.z; z++) {
      size_t index = Coord2ToArrayIndex(side, z, y);
      if (slots[index])
        continue;
      slots[index] = true;

      // We check if the voxel below exists. If it does, then this
      // face is not visible.
      if (GetVoxelElement(x_to_check, y, z))
        continue;

      // This face is visible.
      Quad3<int> face;
      face.min = {x, y, z};
      face.max = {x, y, z};

      // We look how bit we can make this grow z-wise.
      for (int iz = z + 1; iz < quad.max.z; iz++) {
        size_t new_index = Coord2ToArrayIndex(side, iz, y);
        if (slots[new_index])
          break;
        slots[new_index] = true;
        if (GetVoxelElement(x_to_check, y, iz))
          break;

        // We can grow the face.
        face.max.z = iz;
      }

      // We see how much we can grow the face y-wise
      bool found_y_extension = true;
      for (int iy = y + 1; iy <= quad.max.y; iy++) {
        for (int iz = z + 1; iz <= quad.max.z; iz++) {
          // We check if the current face is visible all the way.
          size_t new_index = Coord2ToArrayIndex(side, iz, iy);
          if (slots[new_index]) {
            found_y_extension = false;
            break;
          }
          slots[new_index] = true;
          if (GetVoxelElement(x_to_check, iy, iz)) {
            found_y_extension = false;
            break;
          }
        }

        if (!found_y_extension)
          break;
        face.max.y = iy;
      }

      // Now we have grown the face as bit as it gets.
      faces.push_back(std::move(face));
    }
  }

  return faces;
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
                    VoxelType::kGrassDirt, VoxelType::kTransparent);
        SetCubeFace(*atlas_, &voxel, Voxel::Face::kFront,
                    VoxelType::kGrassDirt, VoxelType::kTransparent);
        SetCubeFace(*atlas_, &voxel, Voxel::Face::kLeft,
                    VoxelType::kGrassDirt, VoxelType::kCrack4);
        SetCubeFace(*atlas_, &voxel, Voxel::Face::kRight,
                    VoxelType::kGrassDirt, VoxelType::kTransparent);
        SetCubeFace(*atlas_, &voxel, Voxel::Face::kTop,
                    VoxelType::kGrass, VoxelType::kCrack9);
        SetCubeFace(*atlas_, &voxel, Voxel::Face::kBottom,
                    VoxelType::kDirt, VoxelType::kTransparent);
      }
    }
  }
  return true;
}

void VoxelChunk::CalculateGreedyMesh() {
  uint64_t before = SDL_GetPerformanceCounter();

  // Get the calculated faces.
  std::vector<TypedFace> faces = CalculateFaces();

  // Put them into separate buckets so we can but them separatedly into the
  // OpenGL buffers.
  std::vector<float> vbo_data;
  vbo_data.reserve(TypedFace::kVertCount * faces.size() +
                   TypedFace::kUVCount * faces.size());

  // We put the vertex data first, then the uv
  for (auto& face : faces) {
    vbo_data.emplace_back(vbo_data.end(), face.verts,
                                          face.verts + TypedFace::kVertCount);
  }
  for (auto& face : faces) {
    vbo_data.emplace_back(vbo_data.end(), face.uvs,
                                          face.uvs + TypedFace::kUVCount);
  }

  // Send the data over to the GPU.

  glBindBuffer(GL_ARRAY_BUFFER, vbo_.value);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(vbo_data), vbo_data.data(),
               GL_STATIC_DRAW);
  // Vertices start at the beginning of the buffer.
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  // UVs start right after.
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                        2 * sizeof(float),
                        (void*)(TypedFace::kVertCount * faces.size()));


  // Calculate the indices
  size_t indices_count = TypedFace::kVertCount * faces.size() / 4;
  std::vector<float> indices;
  indices.reserve(indices_count);
  for (size_t base = 0;
       base < indices_count;
       base += 4) {
    indices.emplace_back(base + 0);
    indices.emplace_back(base + 1);
    indices.emplace_back(base + 2);
    indices.emplace_back(base + 2);
    indices.emplace_back(base + 1);
    indices.emplace_back(base + 3);
  }
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_.value);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               sizeof(indices), indices.data(), GL_STATIC_DRAW);

  if (CHECK_GL_ERRORS("Greedy mesh sending over to the GPU"))
    exit(1);

  uint64_t after = SDL_GetPerformanceCounter();
  uint64_t frequency = SDL_GetPerformanceFrequency();
  uint64_t delta = after - before;
  float time = (float)((double)delta / frequency);
  LOG(DEBUG) << "----------------------\n\n\n";
  LOG(DEBUG) << "Meshing timing: " << time * 1000.0f << " ms.";

}

void
VoxelChunk::Render(Shader* shader) {
  if (!greedy) {
    return;
    for (Voxel& voxel : voxels_) voxel.Render(shader);
  } else {
    glBindVertexArray(vao_.value);

    // TODO(donosoc): Do this only when needed.
    shader->SetMat4(Shader::Attributes::kModel, glm::mat4(1.0f));

    std::vector<float> vertices;
    std::vector<float> uvs;
    for (auto& quad : quads_) {
      auto meshi = CalculateMeshFromFace(*atlas_, quad);
      vertices.push_back(vertices.end(), meshi.vertices.begin(), meshi.vertices.end());
      uvs.push_back(uvs.end(), meshi.uvs.begin(), meshi.uvs.end());

      uint32_t base =





    }

    for (auto& z_quads : quads_) {
      for (VoxelTypeQuad& typed_quad : z_quads) {
        auto meshi = CalculateMeshFromQuads(*atlas_, {0, 0, 0}, typed_quad);
        auto meshi = CalculateMeshFromFace(*atlas_,

        glBindBuffer(GL_ARRAY_BUFFER, vbo_.value);
        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(float) * meshi.vertices.size(),
                     meshi.vertices.data(),
                     GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, ebo_.value);
        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(float) * meshi.indices.size(),
                     meshi.indices.data(),
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
