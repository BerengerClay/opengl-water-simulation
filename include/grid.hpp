#pragma once
#include <GL/glew.h>
#include <tuple>
#include <vector>

std::tuple<GLuint, GLuint, int> createGridVAO(int N, float step);
