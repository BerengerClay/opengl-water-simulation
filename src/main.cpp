// src/main.cpp
#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.hpp"
#include "grid.hpp"
#include "simulation.hpp"
#include "shader_utils.hpp"

#include <iostream>
#include <vector>
#include <algorithm>

// Macro pour capturer les erreurs OpenGL
#define TEST_OPENGL_ERROR()                                      \
  do                                                             \
  {                                                              \
    GLenum err = glGetError();                                   \
    if (err != GL_NO_ERROR)                                      \
      std::cerr << "OpenGL ERROR at line " << __LINE__ << ": 0x" \
                << std::hex << err << std::dec << std::endl;     \
  } while (0)

// ---- Configuration ----
static const int N = 128;
static const float STEP = 1.0f;
static const float DT = 0.016f;
static const float DAMPING = 0.995f;
static const float IMPACT_AMP = -2.0f;
static const int IMPACT_FRAMES = 20;
static const float DROP_RADIUS = 5.0f;
static const float HEIGHT_SCALE = 1.5f;
static const float SPHERE_SIZE = 0.2f;

// ---- Globals ----
Simulation sim(N, STEP, DT, DAMPING);

GLuint waterProgram = 0, dropProgram = 0;
GLuint waterVAO = 0, waterVBO = 0;
int waterCount = 0;
GLuint heightTex = 0, velocityTex = 0;

GLuint sphereVAO = 0, sphereVBO = 0, sphereEBO = 0;
int sphereCount = 0;

Camera camera(20.0f, -90.0f, 30.0f);
bool dragging = false;
int lastX = 0, lastY = 0;

bool impacting = false;
int impactFrame = 0;
glm::vec3 dropPos;

// --- Boat position ---
glm::vec3 boatPos(0.0f, 0.0f, 0.0f);
glm::vec3 prevBoatPos = boatPos;
float boatHeading = 0.0f; // in radians, 0 = facing +Z

// Forward declarations
void init_glut(int &argc, char **argv);
bool init_glew();
void init_GL();
void init_shaders();
void init_water_mesh_and_texture();
void init_drop_mesh();
void window_resize(int w, int h);
void display();
void idle();
void mouse_button(int button, int state, int x, int y);
void mouse_move(int x, int y);
void keyboard(unsigned char key, int x, int y);

// ---- Main ----
int main(int argc, char **argv)
{
  init_glut(argc, argv);
  if (!init_glew())
  {
    std::cerr << "GLEW init failed\n";
    return -1;
  }
  init_GL();
  init_shaders();
  init_water_mesh_and_texture();
  init_drop_mesh();
  glutMainLoop();
  return 0;
}

// ---- GLUT initialization ----
void init_glut(int &argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitContextVersion(3, 3);
  glutInitContextProfile(GLUT_CORE_PROFILE | GLUT_DEBUG);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
  glutInitWindowSize(1280, 720);
  glutCreateWindow("Simulation d'eau");
  glutDisplayFunc(display);
  glutReshapeFunc(window_resize);
  glutIdleFunc(idle);
  glutMouseFunc(mouse_button);
  glutMotionFunc(mouse_move);
  glutKeyboardFunc(keyboard); // Add this line
}

// ---- GLEW initialization ----
bool init_glew()
{
  glewExperimental = GL_TRUE;
  return glewInit() == GLEW_OK;
}

// ---- OpenGL settings ----
void init_GL()
{
  glEnable(GL_DEPTH_TEST);
  TEST_OPENGL_ERROR();
  glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
  TEST_OPENGL_ERROR();
}

// ---- Shader compilation ----
void init_shaders()
{
  waterProgram = createShaderProgram("../shaders/wave.vert",
                                     "../shaders/wave.frag");
  dropProgram = createShaderProgram("../shaders/drop.vert",
                                    "../shaders/drop.frag");
}

// ---- Water mesh & height texture ----
void init_water_mesh_and_texture()
{
  std::tie(waterVAO, waterVBO, waterCount) = createGridVAO(N, STEP);

  glGenTextures(1, &heightTex);
  TEST_OPENGL_ERROR();
  glBindTexture(GL_TEXTURE_2D, heightTex);
  TEST_OPENGL_ERROR();
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, N + 1, N + 1, 0,
               GL_RED, GL_FLOAT, nullptr);
  TEST_OPENGL_ERROR();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  TEST_OPENGL_ERROR();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  TEST_OPENGL_ERROR();
  glBindTexture(GL_TEXTURE_2D, 0);

  glGenTextures(1, &velocityTex);
  glBindTexture(GL_TEXTURE_2D, velocityTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, N + 1, N + 1, 0,
               GL_RED, GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);
}

// ---- Sphere mesh for drop indicator ----
void init_drop_mesh()
{
  std::vector<glm::vec3> verts;
  std::vector<unsigned int> inds;
  int sectors = 24, stacks = 24;
  for (int i = 0; i <= stacks; ++i)
  {
    float lat = glm::half_pi<float>() - i * glm::pi<float>() / stacks;
    float xy = cos(lat), z = sin(lat);
    for (int j = 0; j <= sectors; ++j)
    {
      float lon = j * 2.f * glm::pi<float>() / sectors;
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
  TEST_OPENGL_ERROR();
  glGenBuffers(1, &sphereVBO);
  TEST_OPENGL_ERROR();
  glGenBuffers(1, &sphereEBO);
  TEST_OPENGL_ERROR();

  glBindVertexArray(sphereVAO);
  TEST_OPENGL_ERROR();

  glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
  TEST_OPENGL_ERROR();
  glBufferData(GL_ARRAY_BUFFER,
               verts.size() * sizeof(glm::vec3),
               verts.data(),
               GL_STATIC_DRAW);
  TEST_OPENGL_ERROR();
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  TEST_OPENGL_ERROR();
  glEnableVertexAttribArray(0);
  TEST_OPENGL_ERROR();

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
  TEST_OPENGL_ERROR();
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               inds.size() * sizeof(unsigned int),
               inds.data(),
               GL_STATIC_DRAW);
  TEST_OPENGL_ERROR();

  glBindVertexArray(0);
}

// ---- Window resize ----
void window_resize(int w, int h)
{
  glViewport(0, 0, w, h);
  TEST_OPENGL_ERROR();
}

// ---- Idle callback ----
void idle()
{
  glutPostRedisplay();
}

// ---- Render loop ----
void display()
{
  // Progressive drop impact
  if (impacting && impactFrame < IMPACT_FRAMES)
  {
    float a = IMPACT_AMP * (impactFrame / float(IMPACT_FRAMES));
    int gx = int((dropPos.x / STEP) + N / 2.0f);
    int gz = int((dropPos.z / STEP) + N / 2.0f);
    sim.addDrop(gx, gz, a, DROP_RADIUS);
    ++impactFrame;
  }
  else
    impacting = false;

  if (boatPos != prevBoatPos) {
    int bx = int((boatPos.x / STEP) + N / 2.0f);
    int bz = int((boatPos.z / STEP) + N / 2.0f);
    float boatWaveAmp = -0.5f; // negative for a "dip", positive for a "bump"
    int boatWaveRadius = 3;    // adjust for effect size
    sim.addDrop(bx, bz, boatWaveAmp, boatWaveRadius);
    prevBoatPos = boatPos;
  }
  
  // Update simulation
  sim.update();

  // Upload height map
  glBindTexture(GL_TEXTURE_2D, heightTex);
  TEST_OPENGL_ERROR();
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, N + 1, N + 1,
                  GL_RED, GL_FLOAT, sim.getHeight().data());
  TEST_OPENGL_ERROR();
  glBindTexture(GL_TEXTURE_2D, 0);

  // Upload velocity map
  glBindTexture(GL_TEXTURE_2D, velocityTex);
  TEST_OPENGL_ERROR();
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, N + 1, N + 1,
                  GL_RED, GL_FLOAT, sim.getVelocity().data());
  TEST_OPENGL_ERROR();
  glBindTexture(GL_TEXTURE_2D, 0);

  // Clear
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Render water
  glUseProgram(waterProgram);
  TEST_OPENGL_ERROR();
  GLint locM = glGetUniformLocation(waterProgram, "model");
  GLint locV = glGetUniformLocation(waterProgram, "view");
  GLint locP = glGetUniformLocation(waterProgram, "projection");
  GLint locH = glGetUniformLocation(waterProgram, "heightMap");
  GLint locS = glGetUniformLocation(waterProgram, "heightScale");

  glm::mat4 M = glm::mat4(1.0f);
  glm::mat4 V = camera.getViewMatrix();
  int ww = glutGet(GLUT_WINDOW_WIDTH),
      hh = glutGet(GLUT_WINDOW_HEIGHT);
  glm::mat4 P = glm::perspective(glm::radians(45.0f),
                                 float(ww) / hh, 0.1f, 500.0f);

  glUniformMatrix4fv(locM, 1, GL_FALSE, glm::value_ptr(M));
  glUniformMatrix4fv(locV, 1, GL_FALSE, glm::value_ptr(V));
  glUniformMatrix4fv(locP, 1, GL_FALSE, glm::value_ptr(P));

  // Essential uniforms
  glUniform1i(glGetUniformLocation(waterProgram, "gridSize"), N);
  glUniform1f(glGetUniformLocation(waterProgram, "step"), STEP);

  glUniform3fv(glGetUniformLocation(waterProgram, "viewPos"), 1, glm::value_ptr(camera.getPosition()));
  glUniform1i(locH, 0);
  glUniform1f(locS, HEIGHT_SCALE);

  float dropProgress = (impactFrame < IMPACT_FRAMES) ? (impactFrame / float(IMPACT_FRAMES)) : 1.0f;
  glUniform1f(glGetUniformLocation(waterProgram, "dropProgress"), dropProgress);

  // Bind textures
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, heightTex);
  glUniform1i(glGetUniformLocation(waterProgram, "heightMap"), 0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, velocityTex);
  glUniform1i(glGetUniformLocation(waterProgram, "velocityMap"), 1);

  glUniform1f(glGetUniformLocation(waterProgram, "threshold1"), 0.05f);
  glUniform1f(glGetUniformLocation(waterProgram, "threshold2"), 0.15f);
  glUniform1f(glGetUniformLocation(waterProgram, "velocityScale"), 0.5f);

  // Draw
  glBindVertexArray(waterVAO);
  glDrawElements(GL_TRIANGLES, waterCount, GL_UNSIGNED_INT, nullptr);
  glBindVertexArray(0);

  // Render drop sphere while impacting
  if (impactFrame < IMPACT_FRAMES)
  {
    glUseProgram(dropProgram);
    TEST_OPENGL_ERROR();
    GLint dM = glGetUniformLocation(dropProgram, "model");
    GLint dV = glGetUniformLocation(dropProgram, "view");
    GLint dP = glGetUniformLocation(dropProgram, "projection");
    glm::mat4 Md = glm::translate(glm::mat4(1.0f), dropPos) * glm::scale(glm::mat4(1.0f), glm::vec3(SPHERE_SIZE));
    glUniformMatrix4fv(dM, 1, GL_FALSE, glm::value_ptr(Md));
    glUniformMatrix4fv(dV, 1, GL_FALSE, glm::value_ptr(V));
    glUniformMatrix4fv(dP, 1, GL_FALSE, glm::value_ptr(P));

    glBindVertexArray(sphereVAO);
    TEST_OPENGL_ERROR();
    glDrawElements(GL_TRIANGLES, sphereCount, GL_UNSIGNED_INT, nullptr);
    TEST_OPENGL_ERROR();
    glBindVertexArray(0);
  }

  // --- Boat position update ---
  int bx = int((boatPos.x / STEP) + N / 2.0f);
  int bz = int((boatPos.z / STEP) + N / 2.0f);
  const auto& heights = sim.getHeight();

  // Compute local axes
  float c = cos(boatHeading);
  float s = sin(boatHeading);
  glm::vec2 forward(s, c);
  glm::vec2 right(c, -s);

  // Center
  float hC = heights[bz * (N + 1) + bx];

  // Forward/back (local X)
  int bxF = int((boatPos.x + STEP * forward.x) / STEP + N / 2.0f);
  int bzF = int((boatPos.z + STEP * forward.y) / STEP + N / 2.0f);
  int bxB = int((boatPos.x - STEP * forward.x) / STEP + N / 2.0f);
  int bzB = int((boatPos.z - STEP * forward.y) / STEP + N / 2.0f);
  float hF = heights[std::clamp(bzF, 0, N) * (N + 1) + std::clamp(bxF, 0, N)];
  float hB = heights[std::clamp(bzB, 0, N) * (N + 1) + std::clamp(bxB, 0, N)];

  // Right/left (local Z)
  int bxR = int((boatPos.x + STEP * right.x) / STEP + N / 2.0f);
  int bzR = int((boatPos.z + STEP * right.y) / STEP + N / 2.0f);
  int bxL = int((boatPos.x - STEP * right.x) / STEP + N / 2.0f);
  int bzL = int((boatPos.z - STEP * right.y) / STEP + N / 2.0f);
  float hR = heights[std::clamp(bzR, 0, N) * (N + 1) + std::clamp(bxR, 0, N)];
  float hL = heights[std::clamp(bzL, 0, N) * (N + 1) + std::clamp(bxL, 0, N)];

  // Compute local pitch and roll
  float dx = (hR - hL) * HEIGHT_SCALE / (2.0f * STEP); // roll
  float dz = (hF - hB) * HEIGHT_SCALE / (2.0f * STEP); // pitch

  float maxAngle = glm::radians(20.0f);
  float roll  = glm::clamp(dx, -maxAngle, maxAngle);
  float pitch = glm::clamp(-dz, -maxAngle, maxAngle);

  // Hull parameters
  float hullLength = 2.5f;
  float hullHeight = 4.0f;   // Increased from 2.0f to 4.0f for more height
  float hullWidth  = 8.0f;

  // Cabin parameters
  float cabinLength = 1.0f;
  float cabinHeight = 2.0f;  // Increased from 1.0f to 2.0f for a taller cabin
  float cabinWidth  = 1.0f;

  // --- Boat transform (hull) ---
  glm::mat4 boatBase = glm::translate(glm::mat4(1.0f), glm::vec3(boatPos.x, hC * HEIGHT_SCALE, boatPos.z))
      * glm::rotate(glm::mat4(1.0f), boatHeading, glm::vec3(0, 1, 0)) // heading
      * glm::rotate(glm::mat4(1.0f), pitch, glm::vec3(1, 0, 0))
      * glm::rotate(glm::mat4(1.0f), roll,  glm::vec3(0, 0, 1));

  glm::mat4 boatModel = boatBase
      * glm::scale(glm::mat4(1.0f), glm::vec3(hullLength, hullHeight, hullWidth));

  glUseProgram(dropProgram);
  glUniformMatrix4fv(glGetUniformLocation(dropProgram, "model"), 1, GL_FALSE, glm::value_ptr(boatModel));
  glUniformMatrix4fv(glGetUniformLocation(dropProgram, "view"), 1, GL_FALSE, glm::value_ptr(V));
  glUniformMatrix4fv(glGetUniformLocation(dropProgram, "projection"), 1, GL_FALSE, glm::value_ptr(P));
  glUniform3f(glGetUniformLocation(dropProgram, "objectColor"), 0.6f, 0.3f, 0.1f); // brown
  glBindVertexArray(sphereVAO);
  glDrawElements(GL_TRIANGLES, sphereCount, GL_UNSIGNED_INT, nullptr);
  glBindVertexArray(0);

  // --- Cabin: place it on top of the hull in local space ---
  float cabinYOffset = (hullHeight + cabinHeight) * 0.5f; // sits on top
  float cabinZOffset = 1.5f; // forward on the boat

  glm::mat4 cabinModel = boatBase
      * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, cabinYOffset, cabinZOffset))
      * glm::scale(glm::mat4(1.0f), glm::vec3(cabinLength, cabinHeight, cabinWidth));

  glUniformMatrix4fv(glGetUniformLocation(dropProgram, "model"), 1, GL_FALSE, glm::value_ptr(cabinModel));
  glUniform3f(glGetUniformLocation(dropProgram, "objectColor"), 0.8f, 0.8f, 0.8f); // light gray
  glBindVertexArray(sphereVAO);
  glDrawElements(GL_TRIANGLES, sphereCount, GL_UNSIGNED_INT, nullptr);
  glBindVertexArray(0);

  glutSwapBuffers();
}

// ---- Mouse button callback, with zoom on wheel ----
void mouse_button(int button, int state, int x, int y)
{
  // Zoom via molette : boutons 3 (up) & 4 (down)
  if (state == GLUT_DOWN && button == 4)
  {
    camera.update(0, 0, -1.0f);
    return;
  }
  if (state == GLUT_DOWN && button == 3)
  {
    camera.update(0, 0, +1.0f);
    return;
  }

  // Drag caméra vs drop
  if (button == GLUT_LEFT_BUTTON)
  {
    if (state == GLUT_DOWN)
    {
      int mods = glutGetModifiers();
      if (mods & GLUT_ACTIVE_CTRL)
      {
        dragging = true;
        lastX = x;
        lastY = y;
      }
      else
      {
        int ww = glutGet(GLUT_WINDOW_WIDTH),
            hh = glutGet(GLUT_WINDOW_HEIGHT);
        float xn = 2.0f * x / ww - 1.0f,
              yn = 1.0f - 2.0f * y / hh;
        glm::mat4 P = glm::perspective(glm::radians(45.0f),
                                       float(ww) / hh, 0.1f, 500.0f);
        glm::mat4 V = camera.getViewMatrix();
        glm::mat4 invPV = glm::inverse(P * V);
        glm::vec4 nearP = invPV * glm::vec4(xn, yn, -1, 1);
        glm::vec4 farP = invPV * glm::vec4(xn, yn, 1, 1);
        nearP /= nearP.w;
        farP /= farP.w;
        glm::vec3 orig = glm::vec3(nearP);
        glm::vec3 dir = glm::normalize(glm::vec3(farP - nearP));
        float t = -orig.y / dir.y;
        dropPos = orig + t * dir;
        impacting = true;
        impactFrame = 0;
      }
    }
    else
    {
      dragging = false;
    }
  }
}

// ---- Mouse motion callback ----
void mouse_move(int x, int y)
{
  if (!dragging)
    return;
  int dx = x - lastX, dy = y - lastY;
  lastX = x;
  lastY = y;
  camera.update(dx * 0.2f, dy * 0.2f, 0.0f);
}

// ---- Keyboard input callback ----
void keyboard(unsigned char key, int x, int y)
{
    float moveStep = 0.5f;
    float turnStep = glm::radians(5.0f); // 5 degrees per press

    switch (key)
    {
        case 'w': // forward
            boatPos.x += moveStep * sin(boatHeading);
            boatPos.z += moveStep * cos(boatHeading);
            break;
        case 's': // backward
            boatPos.x -= moveStep * sin(boatHeading);
            boatPos.z -= moveStep * cos(boatHeading);
            break;
        case 'a': // turn left
            boatHeading += turnStep;
            break;
        case 'd': // turn right
            boatHeading -= turnStep;
            break;
    }
}
