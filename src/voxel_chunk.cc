// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/voxel_chunk.h"

#include <bitset>
#include "src/debug/timer.h"
#include "src/graphics/GL/utils.h"
// TODO(Cristian): Remove this dependency.
#include "src/sdl2/def.h"
#include "src/shader.h"
#include "src/texture_array.h"
#include "src/texture_atlas.h"
#include "src/utils/coords.h"
#include "src/utils/glm_impl.h"
#include "src/utils/macros.h"

namespace warhol {

namespace {

static VoxelElement invalid_element = {
  VoxelType::kNone
};

template <typename It>
inline It
AddToContainer(It it, std::initializer_list<float> elems) {
  for (float elem : elems) {
    *it++ = elem;
  }
  return it;
}

}  // namespace

static std::vector<TypedFace>
CalculateFacesFromVoxels(const TextureAtlas&,
                         const std::vector<ExpandedVoxel>&);



VoxelChunk::VoxelChunk() = default;
VoxelChunk::VoxelChunk(TextureAtlas* atlas) : atlas_(atlas) {}

bool VoxelChunk::Init() {
  for (VoxelElement& voxel : elements_) {
    voxel.type = VoxelType::kDirt;
  }
  /* size_t index = Coord3ToArrayIndex(kVoxelChunkSize, 1, 1, 0); */
  /* elements_[index].type = VoxelType::kNone; */

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

  // Vertices.
  glBindBuffer(GL_ARRAY_BUFFER, vbo_.value);
  if (CHECK_GL_ERRORS("Buffering vertices"))
    exit(1);

  // Indices
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_.value);

  glBindVertexArray(NULL);
  glBindBuffer(GL_ARRAY_BUFFER, NULL);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);

  if (CHECK_GL_ERRORS("Unbinding"))
    exit(1);

  return true;
}

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

void VoxelChunk::CalculateMesh() {
  FUNCTION_TIMER();

  // Get the calculated faces.
  faces_ = CalculateFaces();

  std::vector<int> texture_indices;

  NAMED_TIMER(inserting_data, "Inserting data");

  // Put them into separate buckets so we can but them separatedly into the
  // OpenGL buffers.
  std::vector<float> vbo_data;
  vbo_data.reserve(TypedFace::kVertCount * faces_.size() +
                   TypedFace::kUVCount * faces_.size());

  // We put the vertex data first, then the uv
  for (auto& face : faces_) {
    vbo_data.insert(vbo_data.end(), face.verts,
                                    face.verts + TypedFace::kVertCount);
    /* auto coord = ArrayIndexToCoord2(kAtlasSide, (int)face.type); */
    /* int index = tex_array_->GetTextureIndex(coord.x, coord.y); */
    /* texture_indices.push_back(index); */
  }



  size_t uvs_start = vbo_data.size();
  for (auto& face : faces_) {
    vbo_data.insert(vbo_data.end(), face.uvs, face.uvs + TypedFace::kUVCount);
  }


  glBindVertexArray(vao_.value);

  inserting_data.End();

  {
    TIMER("Send data over to GPU");

    // Send the data over to the GPU.
    glBindBuffer(GL_ARRAY_BUFFER, vbo_.value);
    if (CHECK_GL_ERRORS("BindBuffer"))
      exit(1);

    glBufferData(GL_ARRAY_BUFFER,
                 vbo_data.size() * sizeof(float),
                 vbo_data.data(),
                 GL_STATIC_DRAW);
    if (CHECK_GL_ERRORS("BufferData"))
      exit(1);

    // Vertices start at the beginning of the buffer.
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    if (CHECK_GL_ERRORS("VBO"))
      exit(1);
    // UVs start right after.
    size_t uv_offset = uvs_start * sizeof(float);
    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(uv_offset));
    glEnableVertexAttribArray(1);

    if (CHECK_GL_ERRORS("VBO"))
      exit(1);
  }

  {
    TIMER("Indices");

    // Calculate the indices
    std::vector<uint32_t> indices;
    size_t index_base = 0;
    for (size_t i = 0; i < faces_.size(); i++) {
      indices.emplace_back(index_base + 0);
      indices.emplace_back(index_base + 1);
      indices.emplace_back(index_base + 2);
      indices.emplace_back(index_base + 2);
      indices.emplace_back(index_base + 1);
      indices.emplace_back(index_base + 3);
      index_base += 4;
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_.value);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indices.size() * sizeof(uint32_t),
                 indices.data(),
                 GL_STATIC_DRAW);

    if (CHECK_GL_ERRORS("Greedy mesh sending over to the GPU"))
      exit(1);
  }
}

std::vector<TypedFace> VoxelChunk::CalculateFaces() {
  FUNCTION_TIMER();
  // We iterate over z and creating the greatest chunks we can.
  std::vector<ExpandedVoxel> expanded_voxels = ExpandVoxels();

  // Get the faces from the expanded voxels.
  std::vector<TypedFace> faces = CalculateFacesFromVoxels(*atlas_,
                                                          expanded_voxels);
  return faces;
}

std::vector<ExpandedVoxel> VoxelChunk::ExpandVoxels() {
  FUNCTION_TIMER();
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
    // TODO(Cristian): Can we update the mask on the fly and not dot 2 passes?
    for (int z = 0; z < side; z++) {
      for (int x = 0; x < side; x++) {

        int index = Coord3ToArrayIndex(side, x, y, z);
        // We found a quad, we see how big of a grouping we can do.
        if (mask[index]) {
          Quad3<int> quad = {};
          quad.min = {x, y, z};
          quad.max = {x, y, z};

          // We look over the X-axis to see how big this chunk is.
          for (int ix = x + 1; ix < side; ix++) {
            int new_index = Coord3ToArrayIndex(side, ix, y, z);
            if (!mask[new_index]) {
              break;
            }

            quad.max.x = ix;
            mask[new_index] = false;
          }

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
              break;
            }

            // We were able to extend the chunk, so we mark it as not available
            // anymore.
            quad.max.z = iz;
            for (int ix = x; ix <= quad.max.x; ix++) {
              int new_index = Coord3ToArrayIndex(side, ix, y, iz);
              mask[new_index] = false;
            }
          }

          // Now we see if we can make it grow upwards.
          bool found_y_extension = true;
          for (int iy = y + 1; iy < side; iy++) {
            for (int iz = quad.min.z; iz <= quad.max.z; iz++) {
              for (int ix = quad.min.x; ix <= quad.max.x; ix++) {
                // We see if in this current x row we could extend all the way.
                int new_index = Coord3ToArrayIndex(side, ix, iy, iz);
                if (!mask[new_index]) {
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
              break;
            }

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

          ExpandedVoxel expanded_voxel;
          expanded_voxel.quad = std::move(quad);
          expanded_voxel.type = VoxelType::kDirt;
          expanded_voxels.push_back(std::move(expanded_voxel));

        }
      }
    }
  }

  return expanded_voxels;
}

std::vector<TypedFace>
CalculateFacesFromVoxels(const TextureAtlas& atlas,
                         const std::vector<ExpandedVoxel>& voxels) {
  FUNCTION_TIMER();
  std::vector<TypedFace> faces;
  faces.reserve(6 * voxels.size());

  for (const ExpandedVoxel& voxel : voxels) {
    auto& quad_min = voxel.quad.min;
    auto& quad_max = voxel.quad.max;
    Vec3 min{(float)quad_min.x, (float)quad_min.y, (float)quad_min.z};
    Vec3 max{(float)quad_max.x, (float)quad_max.y, (float)quad_max.z};
    max += {1.0f, 1.0f, 1.0f};

    auto uvs = atlas.GetUVs(static_cast<uint8_t>(voxel.type));
    TypedFace face;
    float* vert_ptr = nullptr;
    float* uvs_ptr = nullptr;

    // X min.
    face = {}; vert_ptr = face.verts; uvs_ptr = face.uvs;
    vert_ptr = AddToContainer(vert_ptr, {min.x, min.y, min.z});
    vert_ptr = AddToContainer(vert_ptr, {min.x, min.y, max.z});
    vert_ptr = AddToContainer(vert_ptr, {min.x, max.y, min.z});
    vert_ptr = AddToContainer(vert_ptr, {min.x, max.y, max.z});
    uvs_ptr = AddToContainer(uvs_ptr, {uvs.min().x, uvs.min().y});
    uvs_ptr = AddToContainer(uvs_ptr, {uvs.max().x, uvs.min().y});
    uvs_ptr = AddToContainer(uvs_ptr, {uvs.min().x, uvs.max().y});
    uvs_ptr = AddToContainer(uvs_ptr, {uvs.max().x, uvs.max().y});
    faces.emplace_back(std::move(face));
    // X max.
    face = {}; vert_ptr = face.verts; uvs_ptr = face.uvs;
    vert_ptr = AddToContainer(vert_ptr, {max.x, min.y, max.z});
    vert_ptr = AddToContainer(vert_ptr, {max.x, min.y, min.z});
    vert_ptr = AddToContainer(vert_ptr, {max.x, max.y, max.z});
    vert_ptr = AddToContainer(vert_ptr, {max.x, max.y, min.z});
    uvs_ptr = AddToContainer(uvs_ptr, {uvs.min().x, uvs.max().y});
    uvs_ptr = AddToContainer(uvs_ptr, {uvs.min().x, uvs.min().y});
    uvs_ptr = AddToContainer(uvs_ptr, {uvs.max().x, uvs.max().y});
    uvs_ptr = AddToContainer(uvs_ptr, {uvs.max().x, uvs.min().y});
    faces.emplace_back(std::move(face));
    // Z min.
    face = {}; vert_ptr = face.verts; uvs_ptr = face.uvs;
    vert_ptr = AddToContainer(vert_ptr, {max.x, min.y, min.z});
    vert_ptr = AddToContainer(vert_ptr, {min.x, min.y, min.z});
    vert_ptr = AddToContainer(vert_ptr, {max.x, max.y, min.z});
    vert_ptr = AddToContainer(vert_ptr, {min.x, max.y, min.z});
    uvs_ptr = AddToContainer(uvs_ptr, {uvs.max().x, uvs.min().y});
    uvs_ptr = AddToContainer(uvs_ptr, {uvs.min().x, uvs.min().y});
    uvs_ptr = AddToContainer(uvs_ptr, {uvs.max().x, uvs.max().y});
    uvs_ptr = AddToContainer(uvs_ptr, {uvs.min().x, uvs.max().y});
    faces.emplace_back(std::move(face));
    // Z max.
    face = {}; vert_ptr = face.verts; uvs_ptr = face.uvs;
    vert_ptr = AddToContainer(vert_ptr, {min.x, min.y, max.z});
    vert_ptr = AddToContainer(vert_ptr, {max.x, min.y, max.z});
    vert_ptr = AddToContainer(vert_ptr, {min.x, max.y, max.z});
    vert_ptr = AddToContainer(vert_ptr, {max.x, max.y, max.z});
    uvs_ptr = AddToContainer(uvs_ptr, {uvs.min().x, uvs.min().y});
    uvs_ptr = AddToContainer(uvs_ptr, {uvs.max().x, uvs.min().y});
    uvs_ptr = AddToContainer(uvs_ptr, {uvs.min().x, uvs.max().y});
    uvs_ptr = AddToContainer(uvs_ptr, {uvs.max().x, uvs.max().y});
    faces.emplace_back(std::move(face));
    // Y min.
    face = {}; vert_ptr = face.verts; uvs_ptr = face.uvs;
    vert_ptr = AddToContainer(vert_ptr, {min.x, min.y, max.z});
    vert_ptr = AddToContainer(vert_ptr, {max.x, min.y, max.z});
    vert_ptr = AddToContainer(vert_ptr, {min.x, min.y, min.z});
    vert_ptr = AddToContainer(vert_ptr, {max.x, min.y, min.z});
    uvs_ptr = AddToContainer(uvs_ptr, {uvs.min().x, uvs.min().y});
    uvs_ptr = AddToContainer(uvs_ptr, {uvs.max().x, uvs.min().y});
    uvs_ptr = AddToContainer(uvs_ptr, {uvs.min().x, uvs.max().y});
    uvs_ptr = AddToContainer(uvs_ptr, {uvs.max().x, uvs.max().y});
    faces.emplace_back(std::move(face));
    // Y max.
    face = {}; vert_ptr = face.verts; uvs_ptr = face.uvs;
    vert_ptr = AddToContainer(vert_ptr, {min.x, max.y, max.z});
    vert_ptr = AddToContainer(vert_ptr, {max.x, max.y, max.z});
    vert_ptr = AddToContainer(vert_ptr, {min.x, max.y, min.z});
    vert_ptr = AddToContainer(vert_ptr, {max.x, max.y, min.z});
    uvs_ptr = AddToContainer(uvs_ptr, {uvs.min().x, uvs.min().y});
    uvs_ptr = AddToContainer(uvs_ptr, {uvs.max().x, uvs.min().y});
    uvs_ptr = AddToContainer(uvs_ptr, {uvs.min().x, uvs.max().y});
    uvs_ptr = AddToContainer(uvs_ptr, {uvs.max().x, uvs.max().y});
    faces.emplace_back(std::move(face));
  }

  return faces;
}


void
VoxelChunk::Render(Shader* shader) {
  // TODO(donosoc): Do this only when needed.
  shader->SetMat4(Shader::Attributes::kModel, glm::mat4(1.0f));

  glBindVertexArray(vao_.value);
  glDrawElements(GL_TRIANGLES, 6 * faces_.size(), GL_UNSIGNED_INT, 0);
}

#if 0

// NOTE(Cristian): This is an experiment to reduce the amount of faces sent
//                 to draw, by expanding the faces once we have the expanded
//                 voxels.

std::vector<Quad3<int>>renderdoc capture glBufferData
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

#endif

}  // namespace warhol
