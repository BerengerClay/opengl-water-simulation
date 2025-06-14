#include "shader_utils.hpp"
#include "grid.hpp"
#include "camera.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>

// Caméra orbitale
Camera camera(5.0f, 45.0f, 20.0f);

// État souris
bool mousePressed = false;
float prevX = 0.0f, prevY = 0.0f;

// Callbacks GLFW
void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
  if (!mousePressed)
    return;

  float dx = xpos - prevX;
  float dy = prevY - ypos; // inversé (haut -> bas = rotation +)

  prevX = xpos;
  prevY = ypos;

  camera.update(dx * 0.1f, dy * 0.1f, 0.0f);
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
  if (button == GLFW_MOUSE_BUTTON_LEFT)
  {
    if (action == GLFW_PRESS)
    {
      mousePressed = true;
      double x, y;
      glfwGetCursorPos(window, &x, &y);
      prevX = static_cast<float>(x);
      prevY = static_cast<float>(y);
    }
    else if (action == GLFW_RELEASE)
    {
      mousePressed = false;
    }
  }
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
  camera.update(0.0f, 0.0f, yoffset);
}

int main()
{
  // Initialisation GLFW + GLEW
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow *window = glfwCreateWindow(800, 600, "Simulateur d'eau réaliste", nullptr, nullptr);
  glfwMakeContextCurrent(window);
  glewExperimental = true;
  glewInit();

  // Callbacks pour la souris
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetScrollCallback(window, scroll_callback);

  // OpenGL settings
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  // Shaders et maillage
  GLuint shaderProgram = createShaderProgram("../shaders/wave.vert", "../shaders/wave.frag");
  GLuint vao;
  int vertexCount;
  std::tie(vao, vertexCount) = createGridVAO(200, 0.05f);

  // Boucle principale
  while (!glfwWindowShouldClose(window))
  {
    float timeValue = glfwGetTime();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgram);

    // Paramètres des vagues Gerstner
    glm::vec2 dirs[3] = {
        glm::normalize(glm::vec2(1.0f, 0.5f)),
        glm::normalize(glm::vec2(0.8f, -0.6f)),
        glm::normalize(glm::vec2(-0.3f, 0.9f))};
    float amps[3] = {0.1f, 0.05f, 0.08f};
    float freqs[3] = {4.0f, 6.0f, 5.0f};
    float speeds[3] = {1.0f, 1.5f, 0.8f};
    float steeps[3] = {0.2f, 0.15f, 0.25f};

    for (int i = 0; i < 3; ++i)
    {
      std::string idx = std::to_string(i);
      glUniform1f(glGetUniformLocation(shaderProgram, ("amplitude[" + idx + "]").c_str()), amps[i]);
      glUniform1f(glGetUniformLocation(shaderProgram, ("frequency[" + idx + "]").c_str()), freqs[i]);
      glUniform1f(glGetUniformLocation(shaderProgram, ("speed[" + idx + "]").c_str()), speeds[i]);
      glUniform1f(glGetUniformLocation(shaderProgram, ("steepness[" + idx + "]").c_str()), steeps[i]);
      glUniform2fv(glGetUniformLocation(shaderProgram, ("direction[" + idx + "]").c_str()), 1, &dirs[i][0]);
    }

    glUniform1f(glGetUniformLocation(shaderProgram, "time"), timeValue);

    // Matrices
    glm::vec3 viewPos = camera.getPosition();
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.f / 600.f, 0.1f, 100.0f);

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, &viewPos[0]);

    // Affichage
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
