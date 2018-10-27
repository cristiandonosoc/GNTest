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
  Camera(SDLContext*, Vec3 pos);

  // Changes the direction to face in to the target.
  void SetTarget(const Vec3&);

  // VIEW ----------------------------------------------------------------------
  // IMPORTANT: Whenever you finish changing these, call UpdateView.
  // TODO(Cristian): Verify if this is the access we want.
  Vec3 pos;

  void SetDirection(const Vec3& new_dir);

  const Vec3& direction() const { return direction_; }

  // If you modify these angles, you should call SetDirectionFromEuler()
  const Vec3 rotation() const { return rotation_; }
  float& pitch() { return rotation_.x; }
  float& yaw() { return rotation_.y; }
  float& roll() { return rotation_.z; }
  void SetDirectionFromEuler(float pitch, float yaw);
  void SetDirectionFromEuler() { SetDirectionFromEuler(pitch(), yaw()); }



  const Vec3& up() const { return up_; }

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

  Vec3 rotation_;

  // Updated by UpdateView.
  Vec3 direction_;
  Vec3 up_;

  glm::mat4 view_;
  glm::mat4 proj_;

  SDLContext* sdl_context_;   // Not owning. Must outlive.
};

}  // namespace warhol
