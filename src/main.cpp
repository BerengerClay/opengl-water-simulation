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

//#define SAVE_RENDER

#define TEST_OPENGL_ERROR()                                                             \
  do {									\
    GLenum err = glGetError();					                        \
    if (err != GL_NO_ERROR) std::cerr << "OpenGL ERROR!" << __LINE__ << std::endl;      \
  } while(0)

// variables globales
static const int N = 128;
static const float STEP = 1.0f;
static const float DT = 0.016f;
static const float DAMPING = 0.995f;
static const float IMPACT_AMP = -1.5f;
static const int IMPACT_FRAMES = 30;
static const float DROP_RADIUS = 5.0f;
static const float HEIGHT_SCALE = 1.0f;
static const float SPHERE_SIZE = 0.2f;

Simulation sim(N, STEP, DT, DAMPING);

GLuint waterProgram = 0, dropProgram = 0, skyProgram = 0, landProgram = 0;
GLuint waterVAO = 0, waterVBO = 0;
int waterCount = 0;
GLuint heightTex = 0, velocityTex = 0;

GLuint sphereVAO = 0, sphereVBO = 0, sphereEBO = 0;
int sphereCount = 0;

GLuint skyVAO = 0;

GLuint landVAO = 0, landVBO = 0, landEBO = 0;
int landIndexCount = 0;

Camera camera(20.0f, -90.0f, 30.0f);
bool dragging = false;
int lastX = 0, lastY = 0;

bool impacting = false;
int impactFrame = 0;
glm::vec3 dropPos;

glm::vec3 boatPos(0.0f, 0.0f, 0.0f);
glm::vec3 prevBoatPos = boatPos;
float boatHeading = 0.0f;

bool boatMovedByUser = false;

glm::vec2 boatVelocity(0.0f, 0.0f);

// Prototypes des fonctions
void init_glut(int &argc, char **argv);
bool init_glew();
void init_GL();
void init_shaders();
void init_water_mesh_and_texture();
void init_drop_mesh();
void init_sky();
void init_land();
void init_land_mesh(float innerRadius, float outerRadius, int segments = 128);
void window_resize(int w, int h);
void display();
void idle();
void mouse_button(int button, int state, int x, int y);
void mouse_move(int x, int y);
void keyboard(unsigned char key, int x, int y);

void init_glut(int &argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitContextVersion(4, 5);
  glutInitContextProfile(GLUT_CORE_PROFILE | GLUT_DEBUG);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
  glutInitWindowSize(1280, 720);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("Simulation d'eau");
  glutDisplayFunc(display);
  glutReshapeFunc(window_resize);
  glutIdleFunc(idle);
  glutMouseFunc(mouse_button);
  glutMotionFunc(mouse_move);
  glutKeyboardFunc(keyboard);
}

bool init_glew()
{
  if (glewInit()) {
    std::cerr << " Error while initializing glew";
    return false;
  }
  return true;
}

void init_GL()
{
  glEnable(GL_DEPTH_TEST);TEST_OPENGL_ERROR();
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);TEST_OPENGL_ERROR();
  glClearColor(0.45f, 0.7f, 1.0f, 1.0f); // light blue sky
  glPixelStorei(GL_UNPACK_ALIGNMENT,1);
  glPixelStorei(GL_PACK_ALIGNMENT,1);
}

void init_shaders()
{
  waterProgram = createShaderProgram("../shaders/wave.vert", "../shaders/wave.frag");
  dropProgram = createShaderProgram("../shaders/drop.vert", "../shaders/drop.frag");
  skyProgram = createShaderProgram("../shaders/sky.vert", "../shaders/sky.frag");
  landProgram = createShaderProgram("../shaders/land.vert", "../shaders/land.frag");
}

void init_water_mesh_and_texture()
{
  std::tie(waterVAO, waterVBO, waterCount) = createGridVAO(N, STEP); TEST_OPENGL_ERROR();

  glGenTextures(1, &heightTex); TEST_OPENGL_ERROR();
  glBindTexture(GL_TEXTURE_2D, heightTex); TEST_OPENGL_ERROR();
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, N + 1, N + 1, 0, GL_RED, GL_FLOAT, nullptr); TEST_OPENGL_ERROR();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); TEST_OPENGL_ERROR();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); TEST_OPENGL_ERROR();
  glBindTexture(GL_TEXTURE_2D, 0); TEST_OPENGL_ERROR();

  glGenTextures(1, &velocityTex); TEST_OPENGL_ERROR();
  glBindTexture(GL_TEXTURE_2D, velocityTex); TEST_OPENGL_ERROR();
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, N + 1, N + 1, 0, GL_RED, GL_FLOAT, nullptr); TEST_OPENGL_ERROR();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); TEST_OPENGL_ERROR();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); TEST_OPENGL_ERROR();
  glBindTexture(GL_TEXTURE_2D, 0); TEST_OPENGL_ERROR();
}

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

  glGenVertexArrays(1, &sphereVAO); TEST_OPENGL_ERROR();
  glGenBuffers(1, &sphereVBO); TEST_OPENGL_ERROR();
  glGenBuffers(1, &sphereEBO); TEST_OPENGL_ERROR();
  glBindVertexArray(sphereVAO); TEST_OPENGL_ERROR();

  glBindBuffer(GL_ARRAY_BUFFER, sphereVBO); TEST_OPENGL_ERROR();
  glBufferData(GL_ARRAY_BUFFER,
               verts.size() * sizeof(glm::vec3),
               verts.data(),
               GL_STATIC_DRAW); TEST_OPENGL_ERROR();
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr); TEST_OPENGL_ERROR();
  glEnableVertexAttribArray(0); TEST_OPENGL_ERROR();

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO); TEST_OPENGL_ERROR();
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               inds.size() * sizeof(unsigned int),
               inds.data(),
               GL_STATIC_DRAW); TEST_OPENGL_ERROR();

  glBindVertexArray(0); TEST_OPENGL_ERROR();
}

void init_sky()
{
  glGenVertexArrays(1, &skyVAO); TEST_OPENGL_ERROR();
}

void init_land()
{
    float waterRadius = N * STEP * 0.5f;
    float landRadius = waterRadius + 60.0f;
    init_land_mesh(waterRadius, landRadius, 128);
}

void init_land_mesh(float innerRadius, float outerRadius, int segments)
{
    std::vector<glm::vec3> verts;
    std::vector<unsigned int> inds;

    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * glm::pi<float>() * i / segments;
        float x = cos(angle), z = sin(angle);
        verts.emplace_back(innerRadius * x, 0.0f, innerRadius * z);
        verts.emplace_back(outerRadius * x, 0.0f, outerRadius * z);
    }
    for (int i = 0; i < segments; ++i) {
        int a = 2 * i, b = 2 * i + 1, c = 2 * i + 2, d = 2 * i + 3;
        inds.insert(inds.end(), {static_cast<unsigned int>(a), static_cast<unsigned int>(b), static_cast<unsigned int>(c),
                                 static_cast<unsigned int>(b), static_cast<unsigned int>(d), static_cast<unsigned int>(c)});
    }
    landIndexCount = inds.size();

    glGenVertexArrays(1, &landVAO);
    glGenBuffers(1, &landVBO);
    glGenBuffers(1, &landEBO);

    glBindVertexArray(landVAO);

    glBindBuffer(GL_ARRAY_BUFFER, landVBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(glm::vec3), verts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, landEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size() * sizeof(unsigned int), inds.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void window_resize(int w, int h)
{
  glViewport(0, 0, w, h); TEST_OPENGL_ERROR();
}

void idle()
{
  glutPostRedisplay();
}

void display()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); TEST_OPENGL_ERROR();

  glm::mat4 M = glm::mat4(1.0f);
  glm::mat4 V = camera.getViewMatrix();
  int ww = glutGet(GLUT_WINDOW_WIDTH), hh = glutGet(GLUT_WINDOW_HEIGHT);
  glm::mat4 P = glm::perspective(glm::radians(45.0f), float(ww) / hh, 0.1f, 500.0f);

  // création de la vague
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

  // mouvement du bateau
  if (boatMovedByUser) {
    int bx = int((boatPos.x / STEP) + N / 2.0f);
    int bz = int((boatPos.z / STEP) + N / 2.0f);
    float boatWaveAmp = -1.0f;
    int boatWaveRadius = 3;
    sim.addDrop(bx, bz, boatWaveAmp, boatWaveRadius);
    prevBoatPos = boatPos;
    boatMovedByUser = false;
  }
  
  sim.update();

  glBindTexture(GL_TEXTURE_2D, heightTex); TEST_OPENGL_ERROR();
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, N + 1, N + 1,
                  GL_RED, GL_FLOAT, sim.getHeight().data()); TEST_OPENGL_ERROR();
  glBindTexture(GL_TEXTURE_2D, 0); TEST_OPENGL_ERROR();

  glBindTexture(GL_TEXTURE_2D, velocityTex); TEST_OPENGL_ERROR();
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, N + 1, N + 1,
                  GL_RED, GL_FLOAT, sim.getVelocity().data()); TEST_OPENGL_ERROR();
  glBindTexture(GL_TEXTURE_2D, 0); TEST_OPENGL_ERROR();
  
  // openGL pour le ciel
  glDepthMask(GL_FALSE); TEST_OPENGL_ERROR();
  glUseProgram(skyProgram); TEST_OPENGL_ERROR();
  glBindVertexArray(skyVAO); TEST_OPENGL_ERROR();
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); TEST_OPENGL_ERROR();
  glBindVertexArray(0); TEST_OPENGL_ERROR();
  glUseProgram(0); TEST_OPENGL_ERROR();
  glDepthMask(GL_TRUE); TEST_OPENGL_ERROR();

  // openGL pour la terre
  glUseProgram(landProgram); TEST_OPENGL_ERROR();
  glUniformMatrix4fv(glGetUniformLocation(landProgram, "model"), 1, GL_FALSE, glm::value_ptr(M)); TEST_OPENGL_ERROR();
  glUniformMatrix4fv(glGetUniformLocation(landProgram, "view"), 1, GL_FALSE, glm::value_ptr(V)); TEST_OPENGL_ERROR();
  glUniformMatrix4fv(glGetUniformLocation(landProgram, "projection"), 1, GL_FALSE, glm::value_ptr(P)); TEST_OPENGL_ERROR();
  glBindVertexArray(landVAO); TEST_OPENGL_ERROR();
  glDrawElements(GL_TRIANGLES, landIndexCount, GL_UNSIGNED_INT, nullptr); TEST_OPENGL_ERROR();
  glBindVertexArray(0); TEST_OPENGL_ERROR();
  glUseProgram(0); TEST_OPENGL_ERROR();

  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); TEST_OPENGL_ERROR();
  // openGL pour l'eau
  glUseProgram(waterProgram); TEST_OPENGL_ERROR();

  glUniformMatrix4fv(glGetUniformLocation(waterProgram, "model"), 1, GL_FALSE, glm::value_ptr(M)); TEST_OPENGL_ERROR();
  glUniformMatrix4fv(glGetUniformLocation(waterProgram, "view"), 1, GL_FALSE, glm::value_ptr(V)); TEST_OPENGL_ERROR();
  glUniformMatrix4fv(glGetUniformLocation(waterProgram, "projection"), 1, GL_FALSE, glm::value_ptr(P)); TEST_OPENGL_ERROR();
  glUniform1f(glGetUniformLocation(waterProgram, "heightScale"), HEIGHT_SCALE); TEST_OPENGL_ERROR();
  glUniform1i(glGetUniformLocation(waterProgram, "gridSize"), N); TEST_OPENGL_ERROR();
  glUniform1f(glGetUniformLocation(waterProgram, "step"), STEP); TEST_OPENGL_ERROR();
  glUniform3fv(glGetUniformLocation(waterProgram, "viewPos"), 1, glm::value_ptr(camera.getPosition())); TEST_OPENGL_ERROR();

  glActiveTexture(GL_TEXTURE0); TEST_OPENGL_ERROR();
  glBindTexture(GL_TEXTURE_2D, heightTex); TEST_OPENGL_ERROR();
  glUniform1i(glGetUniformLocation(waterProgram, "heightMap"), 0); TEST_OPENGL_ERROR();

  glActiveTexture(GL_TEXTURE1); TEST_OPENGL_ERROR();
  glBindTexture(GL_TEXTURE_2D, velocityTex); TEST_OPENGL_ERROR();
  glUniform1i(glGetUniformLocation(waterProgram, "velocityMap"), 1); TEST_OPENGL_ERROR();

  glBindVertexArray(waterVAO); TEST_OPENGL_ERROR();
  glDrawElements(GL_TRIANGLES, waterCount, GL_UNSIGNED_INT, nullptr); TEST_OPENGL_ERROR();
  glBindVertexArray(0); TEST_OPENGL_ERROR();

  // openGL pour les vagues
  if (impactFrame < IMPACT_FRAMES)
  {
    glUseProgram(dropProgram); TEST_OPENGL_ERROR();
    glm::mat4 Md = glm::translate(glm::mat4(1.0f), dropPos) * glm::scale(glm::mat4(1.0f), glm::vec3(SPHERE_SIZE)); TEST_OPENGL_ERROR();
    glUniformMatrix4fv(glGetUniformLocation(dropProgram, "model"), 1, GL_FALSE, glm::value_ptr(Md)); TEST_OPENGL_ERROR();
    glUniformMatrix4fv(glGetUniformLocation(dropProgram, "view"), 1, GL_FALSE, glm::value_ptr(V)); TEST_OPENGL_ERROR();
    glUniformMatrix4fv(glGetUniformLocation(dropProgram, "projection"), 1, GL_FALSE, glm::value_ptr(P)); TEST_OPENGL_ERROR();

    glBindVertexArray(sphereVAO); TEST_OPENGL_ERROR();
    glDrawElements(GL_TRIANGLES, sphereCount, GL_UNSIGNED_INT, nullptr); TEST_OPENGL_ERROR();
    glBindVertexArray(0); TEST_OPENGL_ERROR();
  }

  // infos sur le bateau
  int bx = int((boatPos.x / STEP) + N / 2.0f);
  int bz = int((boatPos.z / STEP) + N / 2.0f);
  const auto& heights = sim.getHeight();
  auto [ux, uz] = sim.getLocalVelocity(bx, bz);
  float boatDrag = 0.02f;
  boatPos.x += ux * boatDrag;
  boatPos.z += uz * boatDrag;

  boatPos.x += boatVelocity.x;
  boatPos.z += boatVelocity.y;

  float halfGrid = (N * STEP) / 2.0f;
  if (boatPos.x < -halfGrid) boatPos.x += N * STEP;
  if (boatPos.x >  halfGrid) boatPos.x -= N * STEP;
  if (boatPos.z < -halfGrid) boatPos.z += N * STEP;
  if (boatPos.z >  halfGrid) boatPos.z -= N * STEP;

  boatVelocity *= 0.99f;

  float c = cos(boatHeading);
  float s = sin(boatHeading);
  glm::vec2 forward(s, c);
  glm::vec2 right(c, -s);

  float hC = heights[bz * (N + 1) + bx];

  int bxF = int((boatPos.x + STEP * forward.x) / STEP + N / 2.0f);
  int bzF = int((boatPos.z + STEP * forward.y) / STEP + N / 2.0f);
  int bxB = int((boatPos.x - STEP * forward.x) / STEP + N / 2.0f);
  int bzB = int((boatPos.z - STEP * forward.y) / STEP + N / 2.0f);
  float hF = heights[std::clamp(bzF, 0, N) * (N + 1) + std::clamp(bxF, 0, N)];
  float hB = heights[std::clamp(bzB, 0, N) * (N + 1) + std::clamp(bxB, 0, N)];

  int bxR = int((boatPos.x + STEP * right.x) / STEP + N / 2.0f);
  int bzR = int((boatPos.z + STEP * right.y) / STEP + N / 2.0f);
  int bxL = int((boatPos.x - STEP * right.x) / STEP + N / 2.0f);
  int bzL = int((boatPos.z - STEP * right.y) / STEP + N / 2.0f);
  float hR = heights[std::clamp(bzR, 0, N) * (N + 1) + std::clamp(bxR, 0, N)];
  float hL = heights[std::clamp(bzL, 0, N) * (N + 1) + std::clamp(bxL, 0, N)];

  float dx = (hR - hL) * HEIGHT_SCALE / (2.0f * STEP);
  float dz = (hF - hB) * HEIGHT_SCALE / (2.0f * STEP);

  float maxAngle = glm::radians(20.0f);
  float roll  = glm::clamp(dx, -maxAngle, maxAngle);
  float pitch = glm::clamp(-dz, -maxAngle, maxAngle);

  float hullLength = 2.5f;
  float hullHeight = 4.0f;
  float hullWidth  = 8.0f;

  float cabinLength = 1.0f;
  float cabinHeight = 2.0f;
  float cabinWidth  = 1.0f;

  glm::mat4 boatBase = glm::translate(glm::mat4(1.0f), glm::vec3(boatPos.x, hC * HEIGHT_SCALE, boatPos.z))
      * glm::rotate(glm::mat4(1.0f), boatHeading, glm::vec3(0, 1, 0))
      * glm::rotate(glm::mat4(1.0f), pitch, glm::vec3(1, 0, 0))
      * glm::rotate(glm::mat4(1.0f), roll,  glm::vec3(0, 0, 1));

  glm::mat4 boatModel = boatBase
      * glm::scale(glm::mat4(1.0f), glm::vec3(hullLength, hullHeight, hullWidth));

  // openGL pour le bateau
  glUseProgram(dropProgram); TEST_OPENGL_ERROR();
  glUniformMatrix4fv(glGetUniformLocation(dropProgram, "model"), 1, GL_FALSE, glm::value_ptr(boatModel)); TEST_OPENGL_ERROR();
  glUniformMatrix4fv(glGetUniformLocation(dropProgram, "view"), 1, GL_FALSE, glm::value_ptr(V)); TEST_OPENGL_ERROR();
  glUniformMatrix4fv(glGetUniformLocation(dropProgram, "projection"), 1, GL_FALSE, glm::value_ptr(P)); TEST_OPENGL_ERROR();
  glUniform3f(glGetUniformLocation(dropProgram, "objectColor"), 0.6f, 0.3f, 0.1f);  TEST_OPENGL_ERROR();
  glBindVertexArray(sphereVAO); TEST_OPENGL_ERROR();
  glDrawElements(GL_TRIANGLES, sphereCount, GL_UNSIGNED_INT, nullptr); TEST_OPENGL_ERROR();
  glBindVertexArray(0); TEST_OPENGL_ERROR();

  float cabinYOffset = (hullHeight + cabinHeight) * 0.5f;
  float cabinZOffset = 1.5f;

  glm::mat4 cabinModel = boatBase
      * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, cabinYOffset, cabinZOffset))
      * glm::scale(glm::mat4(1.0f), glm::vec3(cabinLength, cabinHeight, cabinWidth));

  glUniformMatrix4fv(glGetUniformLocation(dropProgram, "model"), 1, GL_FALSE, glm::value_ptr(cabinModel)); TEST_OPENGL_ERROR();
  glUniform3f(glGetUniformLocation(dropProgram, "objectColor"), 0.8f, 0.8f, 0.8f); TEST_OPENGL_ERROR();
  glBindVertexArray(sphereVAO); TEST_OPENGL_ERROR();
  glDrawElements(GL_TRIANGLES, sphereCount, GL_UNSIGNED_INT, nullptr); TEST_OPENGL_ERROR();
  glBindVertexArray(0); TEST_OPENGL_ERROR();

  glutSwapBuffers(); TEST_OPENGL_ERROR();
}

void mouse_button(int button, int state, int x, int y)
{
  if (state == GLUT_DOWN && button == 4)
  {
    camera.update(0, 0, -2.0f);
    return;
  }
  if (state == GLUT_DOWN && button == 3)
  {
    camera.update(0, 0, +2.0f);
    return;
  }

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

void mouse_move(int x, int y)
{
  if (!dragging)
    return;
  int dx = x - lastX, dy = y - lastY;
  lastX = x;
  lastY = y;
  camera.update(dx * 0.2f, dy * 0.2f, 0.0f);
}

void keyboard(unsigned char key, int x, int y)
{
    float accel = 0.1f;
    float turnStep = glm::radians(5.0f);

    switch (key)
    {
        case 'w':
            boatVelocity.x += accel * sin(boatHeading);
            boatVelocity.y += accel * cos(boatHeading);
            boatMovedByUser = true;
            break;
        case 's':
            boatVelocity.x -= accel * sin(boatHeading);
            boatVelocity.y -= accel * cos(boatHeading);
            boatMovedByUser = true;
            break;
        case 'a':
            boatHeading += turnStep;
            break;
        case 'd':
            boatHeading -= turnStep;
            break;
    }
    float maxSpeed = 0.3f;
    float speed = glm::length(boatVelocity);
    if (speed > maxSpeed) {
        boatVelocity = (boatVelocity / speed) * maxSpeed;
    }
}

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
  init_sky();
  init_land();
  glutMainLoop();
  return 0;
}