// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/camera.h"

#include "src/math/angles.h"
#include "src/sdl2/sdl_context.h"
#include "src/shader.h"
#include "src/utils/glm_impl.h"

namespace warhol {

Camera::Camera(SDLContext* sdl_context, glm::vec3 pos, glm::vec3 target)
    : pos(std::move(pos)),
      target(std::move(target)),
      sdl_context_(sdl_context) {
  UpdateView();
  UpdateProjection();
}

void
Camera::SetTarget(Vec3<float> target) {
  direction_ = {target.x - pos.x, target.y - pos.y, target.z - pos.z};
  direction_ = glm::normalize(direction_);
}

void Camera::DirectionFromEuler() {
  direction_.x = cos(glm::radians(pitch())) * cos(glm::radians(yaw()));
  direction_.y = sin(glm::radians(pitch()));
  direction_.z = cos(glm::radians(pitch())) * sin(glm::radians(yaw()));
  direction_ = glm::normalize(direction_);
}

void Camera::EulerFromDirection() {
  // Assumes direction is normalized.
  pitch() = radian2deg(asin(direction_.y));
  yaw() = radian2deg(atan(direction_.z / direction_.x));
}

void Camera::UpdateView() {
  RecalculateCoordVectors();
  view_ = glm::lookAt(pos, pos + direction_, up_);
}

void Camera::RecalculateCoordVectors() {
  // From the direction we calculate the right and up vectors.
  glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
  glm::vec3 camera_right = glm::normalize(glm::cross(up, direction_));
  up_= glm::cross(direction_, camera_right);
}



void Camera::UpdateProjection() {
  proj_ = glm::perspective(
      glm::radians(fov),
      (float)sdl_context_->width() / (float)sdl_context_->height(),
      near_plane,
      far_plane);
}

void Camera::SetView(Shader* shader) const {
  shader->SetMat4("view", view_);
}

void Camera::SetProjection(Shader* shader) const {
  shader->SetMat4("projection", proj_);
}

}  // namespace warhol
