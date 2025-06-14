#include "grid.hpp"
#include <vector>
#include <glm/glm.hpp>

std::tuple<GLuint, int> createGridVAO(int gridSize, float step)
{
  std::vector<glm::vec3> vertices;
  std::vector<glm::vec3> normals;
  std::vector<unsigned int> indices;

  int count = gridSize + 1;
  vertices.resize(count * count);
  normals.resize(count * count, glm::vec3(0.0f));

  for (int i = 0; i <= gridSize; ++i)
  {
    for (int j = 0; j <= gridSize; ++j)
    {
      float x = (i - gridSize / 2) * step;
      float z = (j - gridSize / 2) * step;
      vertices[i * count + j] = glm::vec3(x, 0.0f, z);
    }
  }

  for (int i = 0; i < gridSize; ++i)
  {
    for (int j = 0; j < gridSize; ++j)
    {
      int row1 = i * count;
      int row2 = (i + 1) * count;

      int i0 = row1 + j;
      int i1 = row2 + j;
      int i2 = row1 + j + 1;
      int i3 = row2 + j + 1;

      indices.push_back(i0);
      indices.push_back(i1);
      indices.push_back(i2);
      indices.push_back(i2);
      indices.push_back(i1);
      indices.push_back(i3);

      glm::vec3 n1 = glm::normalize(glm::cross(vertices[i1] - vertices[i0], vertices[i2] - vertices[i0]));
      glm::vec3 n2 = glm::normalize(glm::cross(vertices[i3] - vertices[i2], vertices[i1] - vertices[i2]));

      normals[i0] += n1;
      normals[i1] += n1 + n2;
      normals[i2] += n1 + n2;
      normals[i3] += n2;
    }
  }
  for (auto &n : normals)
    n = glm::normalize(n);

  GLuint vao, vbo[2], ebo;
  glGenVertexArrays(1, &vao);
  glGenBuffers(2, vbo);
  glGenBuffers(1, &ebo);
  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
  glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

  return {vao, static_cast<int>(indices.size())};
}
