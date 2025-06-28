#include "camera.hpp"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

Camera::Camera(float radius, float yaw, float pitch)
    : radius(radius), yaw(yaw), pitch(pitch) {}

void Camera::update(float dx, float dy, float scroll)
{
  yaw += dx;
  pitch = std::clamp(pitch + dy, -89.0f, 89.0f);
  radius = std::clamp(radius - scroll, 1.0f, 100.0f);
}

glm::vec3 Camera::getPosition() const
{
  float ry = glm::radians(yaw), rp = glm::radians(pitch);
  return glm::vec3(
      radius * cos(rp) * cos(ry),
      radius * sin(rp),
      radius * cos(rp) * sin(ry));
}

glm::mat4 Camera::getViewMatrix() const
{
  return glm::lookAt(getPosition(), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}
