// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdlib.h>

#include "src/math/vec.h"
#include "src/utils/glm.h"

namespace warhol {

class SDLContext;
class Shader;

class Camera {
 public:
  // By default points to origin without rotation.
  Camera(SDLContext*, glm::vec3 pos, glm::vec3 target = {});

  void SetTarget(Vec3<float> target);

  // VIEW ----------------------------------------------------------------------
  // IMPORTANT: Whenever you finish changing these, call UpdateView.
  // TODO(Cristian): Verify if this is the access we want.
  glm::vec3 pos;
  glm::vec3 target;
  Vec3<float> rotation;
  float& pitch() { return rotation.x; }
  float& yaw() { return rotation.y; }
  float& roll() { return rotation.z; }

  void DirectionFromEulerAngles();

  const glm::vec3& direction() const { return direction_; }
  const glm::vec3& up() const { return up_; }

  // Call RecalculateCoordVectors before this.
  void UpdateView();
  void SetView(Shader*) const;

  // PROJECTION ----------------------------------------------------------------
  // IMPORTANT: Whenever you finish changing these, call UpdateProjection.
  // TODO(Cristian): Verify if this is the access we want.

  float fov = 45.0f;
  // Sadly, Windef.h has defines for near and far. Because no one would ever use
  // those words right?
  float near_plane = 0.1f;
  float far_plane = 100.0f;
  const glm::mat4 proj() const { return proj_; }

  void UpdateProjection();
  void SetProjection(Shader*) const;

 private:
  void RecalculateCoordVectors();

  // Updated by UpdateView.
  glm::vec3 direction_;
  glm::vec3 up_;

  glm::mat4 view_;
  glm::mat4 proj_;

  SDLContext* sdl_context_;   // Not owning. Must outlive.
};

}  // namespace warhol
