#include "shader_utils.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

static std::string readFile(const char *path)
{
  std::ifstream file(path);
  std::stringstream ss;
  ss << file.rdbuf();
  return ss.str();
}

GLuint compileShader(GLenum type, const char *sourcePath)
{
  std::string src = readFile(sourcePath);
  const char *cstr = src.c_str();
  GLuint sh = glCreateShader(type);
  glShaderSource(sh, 1, &cstr, nullptr);
  glCompileShader(sh);
  GLint ok;
  glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
  if (!ok)
  {
    char buf[512];
    glGetShaderInfoLog(sh, 512, nullptr, buf);
    std::cerr << "Shader compile error:\\n"
              << buf << std::endl;
  }
  return sh;
}

GLuint createShaderProgram(const char *vertPath, const char *fragPath)
{
  GLuint vs = compileShader(GL_VERTEX_SHADER, vertPath);
  GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragPath);
  GLuint pr = glCreateProgram();
  glAttachShader(pr, vs);
  glAttachShader(pr, fs);
  glLinkProgram(pr);
  GLint ok;
  glGetProgramiv(pr, GL_LINK_STATUS, &ok);
  if (!ok)
  {
    char buf[512];
    glGetProgramInfoLog(pr, 512, nullptr, buf);
    std::cerr << "Program link error:\\n"
              << buf << std::endl;
  }
  glDeleteShader(vs);
  glDeleteShader(fs);
  return pr;
}
