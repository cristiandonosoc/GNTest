// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/camera.h"

// TODO(Cristian): Do this include more decent.
BEGIN_IGNORE_WARNINGS()
#include <third_party/include/glm/glm.hpp>
#include <third_party/include/glm/gtc/matrix_transform.hpp>
#include <third_party/include/glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
END_IGNORE_WARNINGS()

#include "src/sdl_context.h"
#include "src/shader.h"

namespace warhol {

Camera::Camera(SDLContext* sdl_context, glm::vec3 pos, glm::vec3 target)
    : pos(std::move(pos)),
      target(std::move(target)),
      sdl_context_(sdl_context) {
  UpdateView();
  UpdateProjection();
}

void Camera::UpdateView() {
  // Update up.
  front_ = glm::normalize(pos - target);
  glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
  glm::vec3 camera_right = glm::normalize(glm::cross(up, front_));
  up_= glm::cross(front_, camera_right);

  view_ = glm::lookAt(pos, target, up_);
}

void Camera::UpdateProjection() {
  // TODO(Cristian): Move this calculation to SDLContext.
  int width, height;
  SDL_GetWindowSize(sdl_context_->window, &width, &height);
  proj_ = glm::perspective(
      glm::radians(fov), (float)width / (float)height, near, far);
}

void Camera::SetView(Shader* shader) const {
  shader->SetMat4("view", view_);
}

void Camera::SetProjection(Shader* shader) const {
  shader->SetMat4("projection", proj_);
}

}  // namespace warhol
