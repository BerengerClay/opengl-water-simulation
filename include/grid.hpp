#pragma once
#include <GL/glew.h>
#include <tuple>
#include <vector>

/// Crée un VAO + VBO dynamique (positions) + EBO, et renvoie (VAO, VBO, indexCount)
std::tuple<GLuint, GLuint, int> createGridVAO(int N, float step);

/// Met à jour le VBO de positions avec les hauteurs fournies
void updateVertexHeights(GLuint vbo, int N, float step, const std::vector<float> &heights);
