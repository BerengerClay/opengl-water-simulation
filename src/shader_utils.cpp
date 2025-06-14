#include "shader_utils.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

std::string readFile(const char *path)
{
  std::ifstream file(path);
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

GLuint compileShader(GLenum type, const char *path)
{
  std::string src = readFile(path);
  const char *srcC = src.c_str();
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &srcC, nullptr);
  glCompileShader(shader);
  return shader;
}

GLuint createShaderProgram(const char *vertPath, const char *fragPath)
{
  GLuint vs = compileShader(GL_VERTEX_SHADER, vertPath);
  GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragPath);
  GLuint prog = glCreateProgram();
  glAttachShader(prog, vs);
  glAttachShader(prog, fs);
  glLinkProgram(prog);
  glDeleteShader(vs);
  glDeleteShader(fs);
  return prog;
}
