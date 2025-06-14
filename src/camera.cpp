#include "camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

Camera::Camera(float radius, float yaw, float pitch)
    : radius(radius), yaw(yaw), pitch(pitch) {}

void Camera::update(float dx, float dy, float scroll)
{
  yaw += dx;
  pitch = std::clamp(pitch + dy, -89.0f, 89.0f);
  radius = std::clamp(radius - scroll, 1.0f, 20.0f);
}

glm::vec3 Camera::getPosition() const
{
  float yawRad = glm::radians(yaw);
  float pitchRad = glm::radians(pitch);
  return glm::vec3(
      radius * cos(pitchRad) * sin(yawRad),
      radius * sin(pitchRad),
      radius * cos(pitchRad) * cos(yawRad));
}

glm::mat4 Camera::getViewMatrix() const
{
  return glm::lookAt(getPosition(), glm::vec3(0.0f), glm::vec3(0, 1, 0));
}
