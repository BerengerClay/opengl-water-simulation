#include "grid.hpp"
#include <vector>
#include <glm/glm.hpp>

std::tuple<GLuint, GLuint, int> createGridVAO(int N, float step)
{
  std::vector<glm::vec3> vertices;
  std::vector<unsigned int> indices;
  vertices.reserve((N + 1) * (N + 1));

  // Création des sommets
  for (int y = 0; y <= N; ++y)
  {
    for (int x = 0; x <= N; ++x)
    {
      float xpos = (x - N / 2.0f) * step;
      float zpos = (y - N / 2.0f) * step;
      vertices.emplace_back(xpos, 0.0f, zpos);
    }
  }
  // Création des indices
  for (int y = 0; y < N; ++y)
  {
    for (int x = 0; x < N; ++x)
    {
      unsigned int i = y * (N + 1) + x;
      indices.insert(indices.end(), {i, i + 1, i + N + 1,
                                     i + 1, i + N + 2, i + N + 1});
    }
  }

  GLuint vao, vbo, ebo;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // VBO positions (dynamic)
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3),
               vertices.data(), GL_DYNAMIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glEnableVertexAttribArray(0);

  // EBO indices
  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               indices.data(), GL_STATIC_DRAW);

  return {vao, vbo, static_cast<int>(indices.size())};
}

void updateVertexHeights(GLuint vbo, int N, float step,
                         const std::vector<float> &heights)
{
  std::vector<glm::vec3> verts;
  verts.reserve((N + 1) * (N + 1));
  for (int y = 0; y <= N; ++y)
  {
    for (int x = 0; x <= N; ++x)
    {
      float xpos = (x - N / 2.0f) * step;
      float ypos = heights[y * (N + 1) + x];
      float zpos = (y - N / 2.0f) * step;
      verts.emplace_back(xpos, ypos, zpos);
    }
  }
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0,
                  verts.size() * sizeof(glm::vec3),
                  verts.data());
}
