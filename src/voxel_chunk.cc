// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/voxel_chunk.h"

#include <assert.h>

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
#include "src/utils/log.h"
#include "src/utils/macros.h"

namespace warhol {

namespace {

static VoxelElement invalid_element = {
  VoxelElement::Type::kNone
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

// VoxelElement ----------------------------------------------------------------

const char* VoxelElement::TypeToString(Type type) {
  switch(type) {
    case VoxelElement::Type::kNone: return "None";
    case VoxelElement::Type::kDirt: return "Dirt";
    case VoxelElement::Type::kGrassDirt: return "GrassDirt";
    case VoxelElement::Type::kCount: break;
  }

  assert(false);
  return "";
}

// Vertex Element Texture Index
#define VETI(type) (float)VoxelType::k##type

// Xmin, Xmax, Zmin, Zmax, Ymin, Ymax
const VoxelElement::FaceTexIndices&
VoxelElement::GetFaceTexIndices(Type type) {
  static std::map<VoxelElement::Type, VoxelElement::FaceTexIndices> map = {
      {VoxelElement::Type::kDirt,
        { VETI(Dirt), VETI(Dirt), VETI(Dirt),
          VETI(Dirt), VETI(Dirt), VETI(Dirt) } },

      {VoxelElement::Type::kGrassDirt,
        { VETI(GrassDirt), VETI(GrassDirt), VETI(GrassDirt),
          VETI(GrassDirt), VETI(Dirt), VETI(Grass) } }
  };

  auto it = map.find(type);
  assert(it != map.end());
  return it->second;
}

// VoxelChunk ------------------------------------------------------------------

std::vector<TypedFace>
CalculateFacesFromVoxels(const std::vector<ExpandedVoxel>& voxels,
                         const std::vector<bool>&);


VoxelChunk::VoxelChunk() = default;
VoxelChunk::VoxelChunk(TextureArray2D* tex_array) : tex_array_(tex_array) {}

bool VoxelChunk::Init() {
  for (VoxelElement& voxel : elements_) {
    voxel.type = VoxelElement::Type::kDirt;
  }

  for (size_t z = 0; z < kVoxelChunkSize; z++) {
    for (size_t x = 0; x < kVoxelChunkSize; x++) {
      size_t index =
          Coord3ToArrayIndex(kVoxelChunkSize, x, kVoxelChunkSize - 1, z);
      elements_[index].type = VoxelElement::Type::kGrassDirt;
    }
  }
  size_t index = Coord3ToArrayIndex(kVoxelChunkSize, 1, 1, 0);
  elements_[index].type = VoxelElement::Type::kNone;

  GetVoxelElement(0, kVoxelChunkSize - 1, 1).type = VoxelElement::Type::kNone;
  GetVoxelElement(0, kVoxelChunkSize - 2, 1).type = VoxelElement::Type::kGrassDirt;

  GL_CALL(glGenVertexArrays, 1, &vao_.value);
  GL_CALL(glBindVertexArray, vao_.value);

  uint32_t buffers[2];
  GL_CALL(glGenBuffers, ARRAY_SIZE(buffers), buffers);
  vbo_ = buffers[0];
  ebo_ = buffers[1];

  // Vertices.
  GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, vbo_.value);

  // Indices
  GL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, ebo_.value);

  GL_CALL(glBindVertexArray, NULL);
  GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, NULL);
  GL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, NULL);

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

VoxelElement& VoxelChunk::GetVoxelElement(int index) {
  return (*this)[index];
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
  vbo_data.reserve(TypedFace::kVertCount * faces_.size() +  // Vert
                   TypedFace::kUVCount * faces_.size() +    // UV
                   4 * faces_.size());                      // Tex index

  // We put the vertex data first, then the uv
  for (auto& face : faces_) {
    vbo_data.insert(vbo_data.end(), face.verts,
                                    face.verts + TypedFace::kVertCount);
  }

  size_t uvs_start = vbo_data.size();
  for (auto& face : faces_) {
    vbo_data.insert(vbo_data.end(), face.uvs, face.uvs + TypedFace::kUVCount);
  }

  size_t tex_index_start = vbo_data.size();
  for (auto& face : faces_) {
    for (size_t i = 0; i < 4; i++)
      vbo_data.emplace_back(face.tex_index);
  }

  inserting_data.End();

  GL_CALL(glBindVertexArray, vao_.value);


  {
    TIMER("Send data over to GPU");

    // Send the data over to the GPU.
    GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, vbo_.value);

    GL_CALL(glBufferData, GL_ARRAY_BUFFER,
                          vbo_data.size() * sizeof(float),
                          vbo_data.data(),
                          GL_STATIC_DRAW);

    // Vertices start at the beginning of the buffer.
    GL_CALL(glVertexAttribPointer,
            0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    GL_CALL(glEnableVertexAttribArray, 0);
    // UVs start right after.
    size_t uv_offset = uvs_start * sizeof(float);
    GL_CALL(glVertexAttribPointer,
            1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(uv_offset));
    GL_CALL(glEnableVertexAttribArray, 1);

    // Tex index
    size_t tex_index_offset = tex_index_start * sizeof(float);
    GL_CALL(glVertexAttribPointer, 2, 1, GL_FLOAT, GL_FALSE,
                                   1 * sizeof(float),
                                   (void*)(tex_index_offset));
    GL_CALL(glEnableVertexAttribArray, 2);
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

    GL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, ebo_.value);
    GL_CALL(glBufferData, GL_ELEMENT_ARRAY_BUFFER,
                          indices.size() * sizeof(uint32_t),
                          indices.data(),
                          GL_STATIC_DRAW);
  }
}

std::vector<TypedFace>
VoxelChunk::CalculateFaces() {
  FUNCTION_TIMER();
  // We iterate over z and creating the greatest chunks we can.
  std::vector<ExpandedVoxel> expanded_voxels = ExpandVoxels();

  // Get the faces from the expanded voxels.
  std::vector<TypedFace> faces =
      CalculateFacesFromVoxels(expanded_voxels, mask_);
  return faces;
}

std::vector<ExpandedVoxel>
VoxelChunk::ExpandVoxels() {
  FUNCTION_TIMER();
  constexpr int side = kVoxelChunkSize;

  // Look over which voxels are there and create one big mask 3D matrix.
  std::bitset<side * side * side> mask;
  mask_.reserve(side * side * side);
  for (int y = 0; y < side; y++) {
    for (int z = 0; z < side; z++) {
      for (int x = 0; x < side; x++) {
        int index = Coord3ToArrayIndex(side, x, y, z);
        mask_[index] = (bool)elements_[index];
        mask[index] = (bool)elements_[index];
      }
    }
  }

  std::vector<ExpandedVoxel> expanded_voxels;

  // Iterate from bottom to top (in our view, that's the Y axis).

  for (int i = 0; i < (int)VoxelElement::Type::kCount; i++) {
    VoxelElement::Type voxel_type = (VoxelElement::Type)i;

    for (int y = 0; y < side; y++) {
      // TODO(Cristian): Can we update the mask on the fly and not dot 2 passes?
      for (int z = 0; z < side; z++) {
        for (int x = 0; x < side; x++) {
          int index = Coord3ToArrayIndex(side, x, y, z);
          auto& voxel_element = GetVoxelElement(index);
          if (mask[index] && voxel_element.type == voxel_type) {
            // We found a quad, we see how big of a grouping we can do.
            mask[index] = false;
            Quad3<int> quad = {};
            quad.min = {x, y, z};
            quad.max = {x, y, z};

            // We look over the X-axis to see how big this chunk is.
            for (int ix = x + 1; ix < side; ix++) {
              int new_index = Coord3ToArrayIndex(side, ix, y, z);
              auto& new_voxel_elem = GetVoxelElement(new_index);
              if (!mask[new_index] || new_voxel_elem.type != voxel_type)
                break;

              quad.max.x = ix;
              mask[new_index] = false;
            }

            // We go over Z row for row to see if this chunk extends
            bool found_z_extension = true;
            for (int iz = z + 1; iz < side; iz++) {
              // If any in this row doesn't match, this quad is not extensible.
              for (int ix = x; ix <= quad.max.x; ix++) {
                int new_index = Coord3ToArrayIndex(side, ix, y, iz);
                auto& new_voxel_elem = GetVoxelElement(new_index);
                if (!mask[new_index] || new_voxel_elem.type != voxel_type) {
                  found_z_extension = false;
                  break;
                }
              }

              // We check if the row did extend the piece.
              // If not, we could not extend and don't mark anything.
              if (!found_z_extension)
                break;

              // We were able to extend the chunk, so we mark it as not
              // available anymore.
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
                  // We see if in this current x row we could extend all the
                  // way.
                  int new_index = Coord3ToArrayIndex(side, ix, iy, iz);
                  auto& new_voxel_elem = GetVoxelElement(new_index);
                  if (!mask[new_index] || new_voxel_elem.type != voxel_type) {
                    found_y_extension = false;
                    break;
                  }
                }

                // We iterated over the current X row for this Z value and we
                // couldn't find the extension.
                if (!found_y_extension)
                  break;
              }

              // At this point, we iterated over the whole "plane" at this Y
              // level and if we didn't find an extension, simply ignore it.
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
            expanded_voxel.type = voxel_type;
            expanded_voxels.push_back(std::move(expanded_voxel));
          }
        }
      }
    }
  }

  return expanded_voxels;
}

std::vector<TypedFace>
CalculateFacesFromVoxels(const std::vector<ExpandedVoxel>& voxels,
                         const std::vector<bool>&) {
  FUNCTION_TIMER();
  std::vector<TypedFace> faces;


  for (const ExpandedVoxel& voxel : voxels) {
    auto& quad_min = voxel.quad.min;
    auto& quad_max = voxel.quad.max;
    Vec3 min{(float)quad_min.x, (float)quad_min.y, (float)quad_min.z};
    Vec3 max{(float)quad_max.x, (float)quad_max.y, (float)quad_max.z};
    max += {1.0f, 1.0f, 1.0f};

    auto tex_indices = VoxelElement::GetFaceTexIndices(voxel.type);

    TypedFace face;
    float* vert_ptr = nullptr;
    float* uvs_ptr = nullptr;

#if 0

    auto x_faces = CalculateFacesX(voxels, mask);
    faces.insert(faces.end(), x_faces.begin(), x_faces.end());
    auto z_faces = CalculateFacesZ(voxels, mask);
    faces.insert(faces.end(), z_faces.begin(), z_faces.end());
    auto y_faces = CalculateFacesY(voxels, mask);
    faces.insert(faces.end(), y_faces.begin(), y_faces.end());

#endif

    /* uvs_ptr = AddToContainer(uvs_ptr, {uvs.min().x, uvs.max().y}); */
    /* uvs_ptr = AddToContainer(uvs_ptr, {uvs.min().x, uvs.min().y}); */
    /* uvs_ptr = AddToContainer(uvs_ptr, {uvs.max().x, uvs.max().y}); */
    /* uvs_ptr = AddToContainer(uvs_ptr, {uvs.max().x, uvs.min().y}); */
    /* uvs_ptr = AddToContainer(uvs_ptr, {0.0f, 0.0f}); */
    /* uvs_ptr = AddToContainer(uvs_ptr, {1.0f, 0.0f}); */
    /* uvs_ptr = AddToContainer(uvs_ptr, {0.0f, 1.0f}); */
    /* uvs_ptr = AddToContainer(uvs_ptr, {1.0f, 1.0f}); */

    /* // X min. */
    /* face = {}; vert_ptr = face.verts; uvs_ptr = face.uvs; */
    /* vert_ptr = AddToContainer(vert_ptr, {min.x, min.y, min.z}); */
    /* vert_ptr = AddToContainer(vert_ptr, {min.x, min.y, max.z}); */
    /* vert_ptr = AddToContainer(vert_ptr, {min.x, max.y, min.z}); */
    /* vert_ptr = AddToContainer(vert_ptr, {min.x, max.y, max.z}); */
    /* uvs_ptr = AddToContainer(uvs_ptr, {min.z, min.y}); */
    /* uvs_ptr = AddToContainer(uvs_ptr, {max.z, min.y}); */
    /* uvs_ptr = AddToContainer(uvs_ptr, {min.z, max.y}); */
    /* uvs_ptr = AddToContainer(uvs_ptr, {max.z, max.y}); */
    /* face.tex_index = tex_indices.x_min; */
    /* faces.emplace_back(std::move(face)); */
    // X max.
    /* face = {}; */
    /* vert_ptr = face.verts; */
    /* uvs_ptr = face.uvs; */
    /* vert_ptr = AddToContainer(vert_ptr, {max.x, min.y, max.z}); */
    /* vert_ptr = AddToContainer(vert_ptr, {max.x, min.y, min.z}); */
    /* vert_ptr = AddToContainer(vert_ptr, {max.x, max.y, max.z}); */
    /* vert_ptr = AddToContainer(vert_ptr, {max.x, max.y, min.z}); */
    /* uvs_ptr = AddToContainer(uvs_ptr, {max.z, min.y}); */
    /* uvs_ptr = AddToContainer(uvs_ptr, {min.z, min.y}); */
    /* uvs_ptr = AddToContainer(uvs_ptr, {max.z, max.y}); */
    /* uvs_ptr = AddToContainer(uvs_ptr, {min.z, max.y}); */
    /* face.tex_index = tex_indices.x_max; */
    /* faces.emplace_back(std::move(face)); */

    // Z min.
    face = {};
    vert_ptr = face.verts;
    uvs_ptr = face.uvs;
    vert_ptr = AddToContainer(vert_ptr, {min.x, min.y, min.z});
    vert_ptr = AddToContainer(vert_ptr, {max.x, min.y, min.z});
    vert_ptr = AddToContainer(vert_ptr, {min.x, max.y, min.z});
    vert_ptr = AddToContainer(vert_ptr, {max.x, max.y, min.z});
    uvs_ptr = AddToContainer(uvs_ptr, {min.x, min.y});
    uvs_ptr = AddToContainer(uvs_ptr, {max.x, min.y});
    uvs_ptr = AddToContainer(uvs_ptr, {min.x, max.y});
    uvs_ptr = AddToContainer(uvs_ptr, {max.x, max.y});
    face.tex_index = tex_indices.z_min;
    faces.emplace_back(std::move(face));
    // Z max.
    face = {}; vert_ptr = face.verts; uvs_ptr = face.uvs;
    vert_ptr = AddToContainer(vert_ptr, {min.x, min.y, max.z});
    vert_ptr = AddToContainer(vert_ptr, {max.x, min.y, max.z});
    vert_ptr = AddToContainer(vert_ptr, {min.x, max.y, max.z});
    vert_ptr = AddToContainer(vert_ptr, {max.x, max.y, max.z});
    uvs_ptr = AddToContainer(uvs_ptr, {min.x, min.y});
    uvs_ptr = AddToContainer(uvs_ptr, {max.x, min.y});
    uvs_ptr = AddToContainer(uvs_ptr, {min.x, max.y});
    uvs_ptr = AddToContainer(uvs_ptr, {max.x, max.y});
    face.tex_index = tex_indices.z_max;
    faces.emplace_back(std::move(face));

    // Y min.
    face = {};
    vert_ptr = face.verts;
    uvs_ptr = face.uvs;
    vert_ptr = AddToContainer(vert_ptr, {min.x, min.y, max.z});
    vert_ptr = AddToContainer(vert_ptr, {max.x, min.y, max.z});
    vert_ptr = AddToContainer(vert_ptr, {min.x, min.y, min.z});
    vert_ptr = AddToContainer(vert_ptr, {max.x, min.y, min.z});
    uvs_ptr = AddToContainer(uvs_ptr, {min.x, max.z});
    uvs_ptr = AddToContainer(uvs_ptr, {max.x, max.z});
    uvs_ptr = AddToContainer(uvs_ptr, {min.x, min.z});
    uvs_ptr = AddToContainer(uvs_ptr, {max.x, min.z});
    face.tex_index = tex_indices.y_min;
    faces.emplace_back(std::move(face));
    // Y max.
    face = {};
    vert_ptr = face.verts;
    uvs_ptr = face.uvs;
    vert_ptr = AddToContainer(vert_ptr, {min.x, max.y, max.z});
    vert_ptr = AddToContainer(vert_ptr, {max.x, max.y, max.z});
    vert_ptr = AddToContainer(vert_ptr, {min.x, max.y, min.z});
    vert_ptr = AddToContainer(vert_ptr, {max.x, max.y, min.z});
    uvs_ptr = AddToContainer(uvs_ptr, {min.x, max.z});
    uvs_ptr = AddToContainer(uvs_ptr, {max.x, max.z});
    uvs_ptr = AddToContainer(uvs_ptr, {min.x, min.z});
    uvs_ptr = AddToContainer(uvs_ptr, {max.x, min.z});
    face.tex_index = tex_indices.y_max;
    faces.emplace_back(std::move(face));
  }

  return faces;
}

void
VoxelChunk::Render(Shader* shader) {
  // TODO(donosoc): Do this only when needed.
  shader->SetMat4(Shader::Uniform::kModel, glm::mat4(1.0f));

  GL_CALL(glBindVertexArray, vao_.value);
  GL_CALL(glDrawElements,
          GL_TRIANGLES, 6 * faces_.size(), GL_UNSIGNED_INT, (void*)0);
}

// NOTE(Cristian): This is an experiment to reduce the amount of faces sent
//                 to draw, by expanding the faces once we have the expanded
//                 voxels.
std::vector<TypedFace>
CalculateFacesX(const std::vector<ExpandedVoxel>& voxels,
                const std::vector<bool>& mask) {
  std::vector<TypedFace> faces;

  for (auto& voxel : voxels) {
    auto& quad = voxel.quad;
    auto& face_tex_indices = VoxelElement::GetFaceTexIndices(voxel.type);

    // Xmin
    int min_x = quad.min.x;
    std::vector<bool> face_mask((quad.max.y - quad.min.y) *
                                (quad.max.z * quad.min.z));
    // This index will track where we are in the face mask.
    int i = -1, ii = -1;
    for (int y = quad.min.y; y <= quad.max.y; y++) {
      for (int z = quad.min.z; z <= quad.max.z; z++) {
        i++;
        size_t min_index = Coord3ToArrayIndex(kVoxelChunkSize, min_x - 1, y, z);
        // If either we have already seen this part of the face or this face is
        // occluded, we ignore it.
        if (face_mask[i] || mask[min_index])
          continue;

        // This face is visible.
        face_mask[i] = true;
        Quad3<int> face_quad = {};
        face_quad.min = {min_x, y, z};
        face_quad.max = {min_x, y, z};

        // We see how much we can make it grow.
        ii = i;
        for (int iz = z + 1; iz <= quad.max.z; iz++) {
          ii++;
          size_t new_index =
              Coord3ToArrayIndex(kVoxelChunkSize, min_x - 1, y, iz);
          if (face_mask[ii] || mask[new_index])
            break;
          face_mask[ii] = true;
          face_quad.max.z++;
        }
        ii = i;
        for (int iy = y + 1; iy <= quad.max.y; iy++) {
          bool found_y_extension = true;
          for (int iz = z + 1; iz <= face_quad.max.z; iz++) {
            ii++;
            size_t new_index =
                Coord3ToArrayIndex(kVoxelChunkSize, min_x - 1, iy, iz);
            if (face_mask[ii] || mask[new_index]) {
              found_y_extension = false;
              break;
            }
            if (!found_y_extension)
              break;
            face_quad.max.y++;
          }
        }

        TypedFace face = {};
        float* vert_ptr = face.verts;
        float* uvs_ptr = face.uvs;

        Vec3 min{(float)face_quad.min.x,
                 (float)face_quad.min.y,
                 (float)face_quad.min.z};
        Vec3 max{(float)(face_quad.max.x + 1),
                 (float)(face_quad.max.y + 1),
                 (float)(face_quad.max.z + 1)};

        vert_ptr = AddToContainer(vert_ptr, {min.x, min.y, min.z});
        vert_ptr = AddToContainer(vert_ptr, {min.x, min.y, max.z});
        vert_ptr = AddToContainer(vert_ptr, {min.x, max.y, min.z});
        vert_ptr = AddToContainer(vert_ptr, {min.x, max.y, max.z});
        uvs_ptr = AddToContainer(uvs_ptr, {min.z, min.y});
        uvs_ptr = AddToContainer(uvs_ptr, {max.z, min.y});
        uvs_ptr = AddToContainer(uvs_ptr, {min.z, max.y});
        uvs_ptr = AddToContainer(uvs_ptr, {max.z, max.y});
        face.tex_index = face_tex_indices.x_min;
        faces.push_back(std::move(face));
      }
    }
  }

  return faces;
}

}  // namespace warhol
