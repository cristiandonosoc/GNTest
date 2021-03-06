// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/camera.h"

#include "warhol/math/math.h"
#include "warhol/sdl2/sdl_context.h"
#include "warhol/shader.h"
#include "warhol/utils/glm_impl.h"

namespace warhol {

Camera::Camera(SDLContext* sdl_context, Vec3 pos)
    : pos(std::move(pos)),
      rotation_({}),
      sdl_context_(sdl_context) {
  UpdateView();
  UpdateProjection();
}

void Camera::SetTarget(const Vec3& target) {
  direction_ = {target.x - pos.x, target.y - pos.y, target.z - pos.z};
  direction_ = direction_.normalize();
  auto [pitch, yaw] = EulerFromDirection(direction_);
  rotation_.x = pitch;
  rotation_.y = yaw;
}

void Camera::SetDirection(const Vec3& new_dir) {
  direction_ = new_dir.normalize();
  auto [pitch, yaw] = EulerFromDirection(direction_);
  rotation_.x = pitch;
  rotation_.y = yaw;
}

void Camera::SetDirectionFromEuler(float npitch, float nyaw) {
  pitch() = npitch;
  yaw() = nyaw;
  roll() = 0;
  direction_ = DirectionFromEuler(npitch, nyaw);
}

void Camera::RecalculateCoordVectors() {
  Vec3 up{0.0f, 1.0f, 0.0f};
  Vec3 right = up.cross(direction_).normalize();
  up_ = direction_.cross(right);
}

void Camera::UpdateView() {
  RecalculateCoordVectors();

  // TODO(Cristian): Move this to our own math framework (or wrap it up).
  glm::vec3 glm_pos = { pos.x, pos.y, pos.z };
  glm::vec3 target = {pos.x + direction_.x,
                      pos.y + direction_.y,
                      pos.z + direction_.z};
  glm::vec3 up = {up_.x, up_.y, up_.z};
  view_ = glm::lookAt(glm_pos, target, up);
}

void Camera::UpdateProjection() {
  proj_ = glm::perspective(
      glm::radians(fov),
      (float)sdl_context_->width() / (float)sdl_context_->height(),
      near_plane,
      far_plane);
}

void Camera::SetView(Shader* shader) const {
  shader->SetMat4(Shader::Uniform::kView, view_);
}

void Camera::SetProjection(Shader* shader) const {
  shader->SetMat4(Shader::Uniform::kProjection, proj_);
}

}  // namespace warhol
