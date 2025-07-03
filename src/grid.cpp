#include "grid.hpp"
#include <vector>
#include <glm/glm.hpp>

std::tuple<GLuint, GLuint, int> createGridVAO(int N, float step)
{
  std::vector<glm::vec3> vertices;
  std::vector<unsigned int> indices;
  vertices.reserve((N + 1) * (N + 1));

  for (int y = 0; y <= N; ++y)
  {
    for (int x = 0; x <= N; ++x)
    {
      float xpos = (x - N / 2.0f) * step;
      float zpos = (y - N / 2.0f) * step;
      vertices.emplace_back(xpos, 0.0f, zpos);
    }
  }
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

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3),
               vertices.data(), GL_DYNAMIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glEnableVertexAttribArray(0);

  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               indices.data(), GL_STATIC_DRAW);

  return {vao, vbo, static_cast<int>(indices.size())};
}
