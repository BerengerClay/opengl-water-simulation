#pragma once
#include <GL/glew.h>
GLuint compileShader(GLenum type, const char *path);
GLuint createShaderProgram(const char *vertPath, const char *fragPath);
