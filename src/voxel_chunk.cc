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

std::vector<TypedFace>
CalculateFacesFromVoxels(const std::vector<ExpandedVoxel>& voxels,
                         const std::vector<bool>&);

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

VoxelChunk::VoxelChunk() = default;

bool VoxelChunk::Init() {
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

  initialized_ = true;
  return true;
}

VoxelElement& VoxelChunk::operator[](int index) {
  if (index < 0 || (size_t)index >= ARRAY_SIZE(elements_))
    return invalid_element;
  return elements_[index];
}

VoxelElement& VoxelChunk::GetVoxel(const Pair3<int>& coord) {
  return GetVoxel(coord.x, coord.y, coord.z);
}

VoxelElement& VoxelChunk::GetVoxel(int x, int y, int z) {
  if (x < 0 || y < 0 || z < 0)
    return invalid_element;
  size_t index = Coord3ToArrayIndex(kVoxelChunkSize, x, y, z);
  if (index >= ARRAY_SIZE(elements_))
    return invalid_element;
  return elements_[index];
}

VoxelElement& VoxelChunk::GetVoxel(int index) {
  return (*this)[index];
}

// CalculateMesh ---------------------------------------------------------------

void VoxelChunk::CalculateMesh() {
  /* FUNCTION_TIMER(); */

  // Get the calculated faces.
  faces_ = CalculateFaces();

  // Put them into separate buckets so we can but them separatedly into the
  // OpenGL buffers.
  std::vector<float> vbo_data;
  vbo_data.reserve(TypedFace::kVertCount * faces_.size() +  // Vert
                   TypedFace::kUVCount * faces_.size() +    // UV
                   4 * faces_.size());                      // Tex index

  // We put the vertex data first, then the uv.
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

  size_t face_color_start = vbo_data.size();
  for (auto& face : faces_) {
    for (size_t i = 0; i < 4; i++) {
      vbo_data.emplace_back(face.face_color[0]);
      vbo_data.emplace_back(face.face_color[1]);
      vbo_data.emplace_back(face.face_color[2]);
    }
  }

  GL_CALL(glBindVertexArray, vao_.value);

  {
    /* TIMER("Send data over to GPU"); */

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

    size_t face_color_offset = face_color_start * sizeof(float);
    (void)face_color_offset;
    /* GL_CALL(glVertexAttribPointer, 3, 3, GL_FLOAT, GL_FALSE, */
    /*                                3 * sizeof(float), */
    /*                                (void*)(face_color_offset)); */
    /* GL_CALL(glEnableVertexAttribArray, 3); */
  }

  {
    /* TIMER("Indices"); */

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
  /* FUNCTION_TIMER(); */
  // We iterate over z and creating the greatest chunks we can.
  std::vector<ExpandedVoxel> expanded_voxels = ExpandVoxels();

  // Get the faces from the expanded voxels.
  std::vector<TypedFace> faces =
      CalculateFacesFromVoxels(expanded_voxels, mask_);
  return faces;
}

std::vector<ExpandedVoxel>
VoxelChunk::ExpandVoxels() {
  /* FUNCTION_TIMER(); */
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
          auto& voxel_element = GetVoxel(index);
          if (mask[index] && voxel_element.type == voxel_type) {
            // We found a quad, we see how big of a grouping we can do.
            mask[index] = false;
            Quad3<int> quad = {};
            quad.min = {x, y, z};
            quad.max = {x, y, z};

            // We look over the X-axis to see how big this chunk is.
            for (int ix = x + 1; ix < side; ix++) {
              int new_index = Coord3ToArrayIndex(side, ix, y, z);
              auto& new_voxel_elem = GetVoxel(new_index);
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
                auto& new_voxel_elem = GetVoxel(new_index);
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
                  auto& new_voxel_elem = GetVoxel(new_index);
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

void
VoxelChunk::Render(Shader* shader, Vec3 offset) {
  // TODO(donosoc): Do this only when needed.
  shader->SetMat4(Shader::Uniform::kModel, glm::mat4(1.0f));

  GL_CALL(glBindVertexArray, vao_.value);
  glm::vec3 v{offset.x, offset.y, offset.z};
  shader->SetMat4(Shader::Uniform::kModel, glm::translate(glm::mat4(1.0f), v));
  GL_CALL(glDrawElements,
          GL_TRIANGLES,
          6 * faces_.size(),
          GL_UNSIGNED_INT,
          (void*)0);
}

namespace {

std::vector<TypedFace>
CalculateFacesX(const ExpandedVoxel& voxel, const std::vector<bool>& mask,
                int x, int x_to_check, int x_offset);

std::vector<TypedFace>
CalculateFacesY(const ExpandedVoxel& voxel, const std::vector<bool>& mask,
                int y, int y_to_check, int y_offset);

std::vector<TypedFace>
CalculateFacesZ(const ExpandedVoxel& voxel, const std::vector<bool>& mask,
                int z, int z_to_check, int z_offset);


std::vector<TypedFace>
CalculateFacesFromVoxels(const std::vector<ExpandedVoxel>& voxels,
                         const std::vector<bool>& mask) {
  std::vector<TypedFace> faces;

  for (auto& voxel : voxels) {
    std::vector<TypedFace> new_faces;

    auto x_min = voxel.quad.min.x;
    new_faces = CalculateFacesX(voxel, mask, x_min, x_min - 1, 0);
    faces.insert(faces.end(), new_faces.begin(), new_faces.end());

    auto x_max = voxel.quad.max.x;
    new_faces = CalculateFacesX(voxel, mask, x_max, x_max + 1, 1);
    faces.insert(faces.end(), new_faces.begin(), new_faces.end());

    auto y_min = voxel.quad.min.y;
    new_faces = CalculateFacesY(voxel, mask, y_min, y_min -1, 0);
    faces.insert(faces.end(), new_faces.begin(), new_faces.end());

    auto y_max = voxel.quad.max.y;
    new_faces = CalculateFacesY(voxel, mask, y_max, y_max + 1, 1);
    faces.insert(faces.end(), new_faces.begin(), new_faces.end());

    auto z_min = voxel.quad.min.z;
    new_faces = CalculateFacesZ(voxel, mask, z_min, z_min - 1, 0);
    faces.insert(faces.end(), new_faces.begin(), new_faces.end());

    auto z_max = voxel.quad.max.z;
    new_faces = CalculateFacesZ(voxel, mask, z_max, z_max + 1, 1);
    faces.insert(faces.end(), new_faces.begin(), new_faces.end());
  }

  return faces;
}

std::vector<TypedFace>
CalculateFacesX(const ExpandedVoxel& voxel,
                const std::vector<bool>& mask,
                int x, int x_to_check, int x_offset) {
  std::vector<TypedFace> faces;
  auto& quad = voxel.quad;
  auto& face_tex_indices = VoxelElement::GetFaceTexIndices(voxel.type);

  int min_y = quad.min.y;
  int min_z = quad.min.z;
  int y_side = quad.max.y - quad.min.y + 1;
  int z_side = quad.max.z - quad.min.z + 1;
  std::vector<bool> face_mask(z_side * y_side);

  // This index will track where we are in the face mask.
  for (int y = quad.min.y; y <= quad.max.y; y++) {
    for (int z = quad.min.z; z <= quad.max.z; z++) {
      int cur_index = z_side * (y - min_y) + (z - min_z);
      int new_index = Coord3ToArrayIndex(kVoxelChunkSize, x_to_check, y, z);

      // If either we have already seen this part of the face or this face is
      // occluded, we ignore it.
      if (face_mask[cur_index] || (new_index >= 0 && mask[new_index]))
        continue;

      face_mask[cur_index] = true;
      Quad3<int> face_quad = {};
      face_quad.min = {x, y, z};
      face_quad.max = {x, y, z};

      // We see how much we can make it grow.
      bool found_z_extension = false;
      for (int iz = z + 1; iz <= quad.max.z; iz++) {
        int ii = z_side * (y - min_y) + (iz - min_z);
        assert(ii >= 0);
        int new_index = Coord3ToArrayIndex(kVoxelChunkSize, x_to_check, y, iz);

        // If we already checked this face or there's a voxel occluding it.
        bool valid = !face_mask[ii] && (new_index < 0 || !mask[new_index]);
        face_mask[ii] = true;
        if (!valid)
          break;
        found_z_extension = true;
        face_quad.max.z++;
      }

      // By default we say that we didn't found a y extension.
      bool found_y_extension = false;
      for (int iy = y + 1; iy <= quad.max.y; iy++) {
        found_y_extension = true;
        for (int iz = z; iz <= face_quad.max.z; iz++) {
          int ii = z_side * (iy - min_y) + (iz - min_z);
          assert(ii >= 0);
          int new_index =
              Coord3ToArrayIndex(kVoxelChunkSize, x_to_check, iy, iz);
          bool valid = !face_mask[ii] && (new_index < 0 || !mask[new_index]);
          if (!valid)
            found_y_extension = false;
          face_mask[ii] = true;
          if (!found_y_extension)
            break;
        }

        if (!found_y_extension)
          break;
        face_quad.max.y++;
      }

      TypedFace face = {};
      float* vert_ptr = face.verts;
      float* uvs_ptr = face.uvs;

      Vec3 min{(float)face_quad.min.x,
               (float)face_quad.min.y,
               (float)face_quad.min.z};
      Vec3 max{(float)(face_quad.max.x),
               (float)(face_quad.max.y),
               (float)(face_quad.max.z)};

      // We offset where this plane should be.
      min.x += x_offset;
      max.x += x_offset;
      max.y += 1;
      max.z += 1;

      vert_ptr = AddToContainer(vert_ptr, {min.x, min.y, min.z});
      vert_ptr = AddToContainer(vert_ptr, {min.x, min.y, max.z});
      vert_ptr = AddToContainer(vert_ptr, {min.x, max.y, min.z});
      vert_ptr = AddToContainer(vert_ptr, {min.x, max.y, max.z});
      uvs_ptr = AddToContainer(uvs_ptr, {min.z, min.y});
      uvs_ptr = AddToContainer(uvs_ptr, {max.z, min.y});
      uvs_ptr = AddToContainer(uvs_ptr, {min.z, max.y});
      uvs_ptr = AddToContainer(uvs_ptr, {max.z, max.y});
      face.tex_index = face_tex_indices.x_min;

      static int face_count = 0;
      face_count++;

      face.face_color[0] = (float)face_count * 0.4f + 0.1f;
      face.face_color[1] = (float)face_count * 2.0f + 0.1f;
      face.face_color[2] = (float)face_count * 1.0f + 0.1f;
      faces.push_back(std::move(face));
    }
  }

  return faces;
}

std::vector<TypedFace>
CalculateFacesY(const ExpandedVoxel& voxel,
                const std::vector<bool>& mask,
                int y_cur, int y_to_check, int y_offset) {
  std::vector<TypedFace> faces;

  auto& quad = voxel.quad;
  auto& face_tex_indices = VoxelElement::GetFaceTexIndices(voxel.type);

  int min_x = quad.min.x;
  int min_z = quad.min.z;
  int x_side = quad.max.x - quad.min.x + 1;
  int z_side = quad.max.z - quad.min.z + 1;
  std::vector<bool> face_mask(x_side * z_side);

  // This index will track where we are in the face mask.
  for (int z = quad.min.z; z <= quad.max.z; z++) {
    for (int x = quad.min.x; x <= quad.max.x; x++) {
      int cur_index = x_side * (z - min_z) + (x - min_x);
      int new_index = Coord3ToArrayIndex(kVoxelChunkSize, x, y_to_check, z);

      // If either we have already seen this part of the face or
      // this face is occluded, we ignore it.
      if (face_mask[cur_index] || (new_index >= 0 && mask[new_index]))
        continue;

      face_mask[cur_index] = true;
      Quad3<int> face_quad = {};
      face_quad.min = {x, y_cur, z};
      face_quad.max = {x, y_cur, z};

      // We see how much we can make it grow.
      bool found_x_extension = false;
      for (int ix = x + 1; ix <= quad.max.x; ix++) {
        int ii = x_side * (z - min_z) + (ix - min_x);
        assert(ii >= 0);
        int new_index = Coord3ToArrayIndex(kVoxelChunkSize, ix, y_to_check, z);

        // If we already checked this face or there's a voxel occluding it.
        bool valid = !face_mask[ii] && (new_index < 0 || !mask[new_index]);
        face_mask[ii] = true;
        if (!valid)
          break;
        found_x_extension = true;
        face_quad.max.x++;
      }

      // By default we say that we didn't found a y extension.
      bool found_z_extension = false;
      for (int iz = z + 1; iz <= quad.max.z; iz++) {
        found_z_extension = true;
        for (int ix = x; ix <= face_quad.max.x; ix++) {
          int ii = x_side * (iz - min_z) + (ix - min_x);
          assert(ii >= 0);
          int new_index =
              Coord3ToArrayIndex(kVoxelChunkSize, ix, y_to_check, iz);

          bool valid = !face_mask[ii] && (new_index < 0 || !mask[new_index]);
          if (!valid)
            found_z_extension = false;
          face_mask[ii] = true;
          if (!found_z_extension)
            break;
        }

        if (!found_z_extension)
          break;
        face_quad.max.z++;
      }

      TypedFace face = {};
      float* vert_ptr = face.verts;
      float* uvs_ptr = face.uvs;

      Vec3 min{(float)face_quad.min.x,
               (float)face_quad.min.y,
               (float)face_quad.min.z};
      Vec3 max{(float)(face_quad.max.x),
               (float)(face_quad.max.y),
               (float)(face_quad.max.z)};

      // We offset where this plane should be.
      max.x += 1;
      min.y += y_offset;
      max.y += y_offset;
      max.z += 1;

      vert_ptr = AddToContainer(vert_ptr, {min.x, min.y, max.z});
      vert_ptr = AddToContainer(vert_ptr, {max.x, min.y, max.z});
      vert_ptr = AddToContainer(vert_ptr, {min.x, min.y, min.z});
      vert_ptr = AddToContainer(vert_ptr, {max.x, min.y, min.z});
      uvs_ptr = AddToContainer(uvs_ptr, {min.x, max.z});
      uvs_ptr = AddToContainer(uvs_ptr, {max.x, max.z});
      uvs_ptr = AddToContainer(uvs_ptr, {min.x, min.z});
      uvs_ptr = AddToContainer(uvs_ptr, {max.x, min.z});
      face.tex_index = face_tex_indices.y_min;

      static int face_count = 0;
      face_count++;

      face.face_color[0] = (float)face_count * 2.4f + 0.1f;
      face.face_color[1] = (float)face_count * 1.2f + 0.1f;
      face.face_color[2] = (float)face_count * 0.9f + 0.1f;
      faces.push_back(std::move(face));
    }
  }

  return faces;
}

std::vector<TypedFace>
CalculateFacesZ(const ExpandedVoxel& voxel,
                const std::vector<bool>& mask,
                int z_cur, int z_to_check, int z_offset) {
  std::vector<TypedFace> faces;

  auto& quad = voxel.quad;
  auto& face_tex_indices = VoxelElement::GetFaceTexIndices(voxel.type);

  int min_x = quad.min.x;
  int min_y = quad.min.y;
  int x_side = quad.max.x - quad.min.x + 1;
  int y_side = quad.max.y - quad.min.y + 1;
  std::vector<bool> face_mask(x_side * y_side);

  // This index will track where we are in the face mask.
  for (int y = quad.min.y; y <= quad.max.y; y++) {
    for (int x = quad.min.x; x <= quad.max.x; x++) {
      int cur_index = x_side * (y - min_y) + (x - min_x);
      int new_index = Coord3ToArrayIndex(kVoxelChunkSize, x, y, z_to_check);

      // If either we have already seen this part of the face or
      // this face is occluded, we ignore it.
      if (face_mask[cur_index] || (new_index >= 0 && mask[new_index]))
        continue;

      face_mask[cur_index] = true;
      Quad3<int> face_quad = {};
      face_quad.min = {x, y, z_cur};
      face_quad.max = {x, y, z_cur};

      // We see how much we can make it grow.
      bool found_x_extension = false;
      for (int ix = x + 1; ix <= quad.max.x; ix++) {
        int ii = x_side * (y - min_y) + (ix - min_x);
        assert(ii >= 0);
        int new_index = Coord3ToArrayIndex(kVoxelChunkSize, ix, y, z_to_check);

        // If we already checked this face or there's a voxel occluding it.
        bool valid = !face_mask[ii] && (new_index < 0 || !mask[new_index]);
        face_mask[ii] = true;
        if (!valid)
          break;
        found_x_extension = true;
        face_quad.max.x++;
      }

      // By default we say that we didn't found a y extension.
      bool found_y_extension = false;
      for (int iy = y + 1; iy <= quad.max.y; iy++) {
        found_y_extension = true;
        for (int ix = x; ix <= face_quad.max.x; ix++) {
          int ii = x_side * (iy - min_y) + (ix - min_x);
          assert(ii >= 0);
          int new_index =
              Coord3ToArrayIndex(kVoxelChunkSize, ix, iy, z_to_check);

          bool valid = !face_mask[ii] && (new_index < 0 || !mask[new_index]);
          if (!valid)
            found_y_extension = false;
          face_mask[ii] = true;
          if (!found_y_extension)
            break;
        }

        if (!found_y_extension)
          break;
        face_quad.max.y++;
      }

      TypedFace face = {};
      float* vert_ptr = face.verts;
      float* uvs_ptr = face.uvs;

      Vec3 min{(float)face_quad.min.x,
               (float)face_quad.min.y,
               (float)face_quad.min.z};
      Vec3 max{(float)(face_quad.max.x),
               (float)(face_quad.max.y),
               (float)(face_quad.max.z)};

      // We offset where this plane should be.
      max.x += 1;
      max.y += 1;
      min.z += z_offset;
      max.z += z_offset;

      vert_ptr = AddToContainer(vert_ptr, {min.x, min.y, max.z});
      vert_ptr = AddToContainer(vert_ptr, {max.x, min.y, max.z});
      vert_ptr = AddToContainer(vert_ptr, {min.x, max.y, max.z});
      vert_ptr = AddToContainer(vert_ptr, {max.x, max.y, max.z});
      uvs_ptr = AddToContainer(uvs_ptr, {min.x, min.y});
      uvs_ptr = AddToContainer(uvs_ptr, {max.x, min.y});
      uvs_ptr = AddToContainer(uvs_ptr, {min.x, max.y});
      uvs_ptr = AddToContainer(uvs_ptr, {max.x, max.y});
      face.tex_index = face_tex_indices.z_max;

      static int face_count = 0;
      face_count++;

      face.face_color[0] = (float)face_count * 0.7f + 0.1f;
      face.face_color[1] = (float)face_count * 1.1f + 0.1f;
      face.face_color[2] = (float)face_count * 1.9f + 0.1f;
      faces.push_back(std::move(face));
    }
  }

  return faces;
}

}  // namespace

}  // namespace warhol
