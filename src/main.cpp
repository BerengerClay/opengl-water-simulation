// src/main.cpp
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.hpp"
#include "grid.hpp"
#include "simulation.hpp"
#include "shader_utils.hpp"

#include <iostream>
#include <vector>

// ---- Configuration ----
const int N = 128;
const float dx = 1.0f;
const float dt = 0.016f;
const float step = dx;
const float damping = 0.995f;
const float dropRadius = 5.0f;
const float sphereSize = 0.2f;
const float startY = 10.0f;
const float gravity = -9.81f;
const int impactFrames = 20;
const float impactAmp = -0.3f;
const float heightScale = 1.0f;

// ---- Globals ----
Simulation sim(N, dx, dt, damping);
GLuint waterVAO = 0, waterVBO = 0;
int waterCount = 0;
GLuint heightTex = 0;

Camera camera(8.0f, -90.0f, 20.0f);
bool mouseDragging = false;
double prevX = 0.0, prevY = 0.0;

// Drop state
bool dropActive = true;
bool impacting = false;
int impactFrame = 0;
glm::vec3 dropPos(0.0f, startY, 0.0f);
glm::vec3 dropVel(0.0f);

// Sphere mesh
GLuint sphereVAO = 0, sphereVBO = 0, sphereEBO = 0;
int sphereCount = 0;

// ---- Forward decl. ----
void mouse_move(GLFWwindow *window, double xpos, double ypos);
void mouse_button(GLFWwindow *window, int button, int action, int mods);
void scroll_cb(GLFWwindow *window, double xoffset, double yoffset);
void createHeightTexture(int size);
void buildSphere(int sectors, int stacks);

// ---- Callbacks & Helpers ----

void mouse_move(GLFWwindow *window, double xpos, double ypos)
{
  if (!mouseDragging)
    return;
  if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS)
    return;
  float ddx = float(xpos - prevX);
  float ddy = float(prevY - ypos);
  prevX = xpos;
  prevY = ypos;
  camera.update(ddx * 0.1f, ddy * 0.1f, 0.0f);
}

void mouse_button(GLFWwindow *window, int button, int action, int mods)
{
  if (button != GLFW_MOUSE_BUTTON_LEFT)
    return;
  if (action == GLFW_PRESS)
  {
    bool ctrl = (mods & GLFW_MOD_CONTROL) != 0;
    double mx, my;
    glfwGetCursorPos(window, &mx, &my);
    if (ctrl)
    {
      // Begin camera drag
      mouseDragging = true;
      prevX = mx;
      prevY = my;
    }
    else
    {
      // Place drop where clicked
      int w, h;
      glfwGetFramebufferSize(window, &w, &h);
      float xndc = (float(mx) / w) * 2.0f - 1.0f;
      float yndc = 1.0f - (float(my) / h) * 2.0f;
      glm::mat4 proj = glm::perspective(glm::radians(45.0f),
                                        float(w) / h, 0.1f, 100.0f);
      glm::mat4 view = camera.getViewMatrix();
      glm::mat4 invPV = glm::inverse(proj * view);

      glm::vec4 nearP = invPV * glm::vec4(xndc, yndc, -1.0f, 1.0f);
      glm::vec4 farP = invPV * glm::vec4(xndc, yndc, 1.0f, 1.0f);
      nearP /= nearP.w;
      farP /= farP.w;

      glm::vec3 origin = glm::vec3(nearP);
      glm::vec3 dir = glm::normalize(glm::vec3(farP - nearP));

      // Intersection with plane y=0
      float t = -origin.y / dir.y;
      glm::vec3 ip = origin + t * dir;

      // Grid indices
      int gx = int((ip.x / step) + N / 2.0f);
      int gz = int((ip.z / step) + N / 2.0f);
      gx = glm::clamp(gx, 0, N);
      gz = glm::clamp(gz, 0, N);

      // Trigger impact progressively
      dropPos = ip;
      dropActive = false;
      impacting = true;
      impactFrame = 0;
      sim.addDrop(gx, gz,
                  impactAmp * (impactFrame / float(impactFrames)),
                  dropRadius);
    }
  }
  else if (action == GLFW_RELEASE)
  {
    mouseDragging = false;
  }
}

void scroll_cb(GLFWwindow *window, double, double yoffset)
{
  camera.update(0.0f, 0.0f, float(yoffset));
}

void createHeightTexture(int size)
{
  glGenTextures(1, &heightTex);
  glBindTexture(GL_TEXTURE_2D, heightTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, size + 1, size + 1, 0,
               GL_RED, GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void buildSphere(int sectors, int stacks)
{
  std::vector<glm::vec3> verts;
  std::vector<unsigned int> inds;
  verts.reserve((stacks + 1) * (sectors + 1));
  for (int i = 0; i <= stacks; ++i)
  {
    float lat = glm::half_pi<float>() - i * glm::pi<float>() / stacks;
    float xy = cos(lat), z = sin(lat);
    for (int j = 0; j <= sectors; ++j)
    {
      float lon = j * 2.0f * glm::pi<float>() / sectors;
      verts.emplace_back(xy * cos(lon), xy * sin(lon), z);
    }
  }
  for (int i = 0; i < stacks; ++i)
  {
    for (int j = 0; j < sectors; ++j)
    {
      unsigned int a = i * (sectors + 1) + j;
      unsigned int b = a + (sectors + 1);
      inds.insert(inds.end(), {a, b, a + 1, a + 1, b, b + 1});
    }
  }
  sphereCount = int(inds.size());

  glGenVertexArrays(1, &sphereVAO);
  glGenBuffers(1, &sphereVBO);
  glGenBuffers(1, &sphereEBO);
  glBindVertexArray(sphereVAO);

  glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
  glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(glm::vec3),
               verts.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size() * sizeof(unsigned int),
               inds.data(), GL_STATIC_DRAW);
}

// ---- Main ----
int main()
{
  if (!glfwInit())
  {
    std::cerr << "GLFW init failed\n";
    return -1;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow *win = glfwCreateWindow(1280, 720, "Water Sim", nullptr, nullptr);
  glfwMakeContextCurrent(win);

  glewExperimental = true;
  if (glewInit() != GLEW_OK)
  {
    std::cerr << "GLEW init failed\n";
    return -1;
  }

  glfwSetCursorPosCallback(win, mouse_move);
  glfwSetMouseButtonCallback(win, mouse_button);
  glfwSetScrollCallback(win, scroll_cb);

  glEnable(GL_DEPTH_TEST);
  glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

  GLuint waterShader = createShaderProgram("../shaders/wave.vert", "../shaders/wave.frag");
  GLuint dropShader = createShaderProgram("../shaders/drop.vert", "../shaders/drop.frag");

  std::tie(waterVAO, waterVBO, waterCount) = createGridVAO(N, step);
  createHeightTexture(N);
  buildSphere(24, 24);

  while (!glfwWindowShouldClose(win))
  {
    glfwPollEvents();

    // Progressive impact
    if (impacting && impactFrame < impactFrames)
    {
      float a = impactAmp * (impactFrame / float(impactFrames));
      // addDrop on the same grid cell as dropPos
      int gx = int((dropPos.x / step) + N / 2.0f);
      int gz = int((dropPos.z / step) + N / 2.0f);
      sim.addDrop(gx, gz, a, dropRadius);
      impactFrame++;
    }
    else
    {
      impacting = false;
    }

    // Update simulation & upload
    sim.update();
    glBindTexture(GL_TEXTURE_2D, heightTex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, N + 1, N + 1, GL_RED, GL_FLOAT, sim.getHeight().data());

    // Render water
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(waterShader);
    glm::mat4 M(1.0f), V = camera.getViewMatrix(),
                       P = glm::perspective(glm::radians(45.0f),
                                            800.0f / 600.0f, 0.1f, 100.0f);
    glm::vec3 vp = camera.getPosition();
    glUniformMatrix4fv(glGetUniformLocation(waterShader, "model"), 1, GL_FALSE, glm::value_ptr(M));
    glUniformMatrix4fv(glGetUniformLocation(waterShader, "view"), 1, GL_FALSE, glm::value_ptr(V));
    glUniformMatrix4fv(glGetUniformLocation(waterShader, "projection"), 1, GL_FALSE, glm::value_ptr(P));
    glUniform3fv(glGetUniformLocation(waterShader, "viewPos"), 1, glm::value_ptr(vp));
    glUniform1i(glGetUniformLocation(waterShader, "heightMap"), 0);
    glUniform1i(glGetUniformLocation(waterShader, "gridSize"), N);
    glUniform1f(glGetUniformLocation(waterShader, "step"), step);
    glUniform1f(glGetUniformLocation(waterShader, "heightScale"), heightScale);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, heightTex);
    glBindVertexArray(waterVAO);
    glDrawElements(GL_TRIANGLES, waterCount, GL_UNSIGNED_INT, 0);

    // Render drop sphere until impact done
    if (impactFrame < impactFrames)
    {
      glUseProgram(dropShader);
      glm::mat4 Md = glm::translate(glm::mat4(1.0f), dropPos) * glm::scale(glm::mat4(1.0f), glm::vec3(sphereSize));
      glUniformMatrix4fv(glGetUniformLocation(dropShader, "model"), 1, GL_FALSE, glm::value_ptr(Md));
      glUniformMatrix4fv(glGetUniformLocation(dropShader, "view"), 1, GL_FALSE, glm::value_ptr(V));
      glUniformMatrix4fv(glGetUniformLocation(dropShader, "projection"), 1, GL_FALSE, glm::value_ptr(P));
      glBindVertexArray(sphereVAO);
      glDrawElements(GL_TRIANGLES, sphereCount, GL_UNSIGNED_INT, 0);
    }

    glfwSwapBuffers(win);
  }

  glfwTerminate();
  return 0;
}
