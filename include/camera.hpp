#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:
  Camera(float radius, float yaw, float pitch);

  void update(float dx, float dy, float scroll);
  glm::vec3 getPosition() const;
  glm::mat4 getViewMatrix() const;

private:
  float radius;
  float yaw;
  float pitch;
};
