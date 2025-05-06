#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <string>

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#include "DeferredRenderer.h"
#include "ObjectReader.h"
#include "LightInfo.h"


class Camera
{
public:
  Camera() {};

  void MouseMovement(float x, float y)
  {
    yaw += .1 * x;
    pitch += .1 * y;

    if (pitch > 89.0f)
      pitch = 89.0f;
    if (pitch < -89.0f)
      pitch = -89.0f;

    Update();
  }
  void forward(float distance) { position = position - distance * backVector; }
  void right(float distance) { position = position + distance * rightVector; }

  glm::mat4 getViewMatrix()
  {
    return glm::lookAt(glm::vec3(position), glm::vec3(position - backVector), glm::vec3(upVector));
  }

  glm::vec4 position = glm::vec4(0.0, 0.0, 5.0, 1.0f);

private:
  void Update() 
  {
    glm::vec3 front = glm::vec3(sin(glm::radians(yaw)) * cos(glm::radians(pitch)), sin(glm::radians(pitch)), -1 * cos(glm::radians(yaw)) * cos(glm::radians(pitch)));
    front = glm::normalize(front);
    backVector = glm::vec4(-1.0f * front, 0.0f);
    rightVector = glm::normalize(glm::vec4(glm::cross(front, glm::vec3(0, 1, 0)), 0.0f));
    upVector = glm::normalize(glm::vec4(glm::cross(glm::vec3(rightVector), front), 0.0f));
  }

  float yaw = 0;
  float pitch = 0;

  glm::vec4 rightVector = glm::vec4(1.0, 0.0, 0.0, 0.0f);
  glm::vec4 upVector = glm::vec4(0.0, 1.0, 0.0, 0.0f);
  glm::vec4 backVector = glm::vec4(0.0, 0.0, 1.0, 0.0f);
};


class Client
{
public:
  int initialize();
  int update();
  int shutdown();

  void mouse(GLFWwindow* window, double xpos, double ypos);
private:
  void GenerateLights(int count);
  void ProcessInput(); 

  DeferredRenderer dRenderer;
  GLFWwindow *window;

  Camera camera;
  glm::mat4 cameraMatrix;
  glm::mat4 projectionMatrix;

  Object centerObject;
  Object sphere;

  float currentTime;

  LightInfo lights;

  bool firstMouse = true;
  float lastX;
  float lastY;

  bool copy = true;
  int mode = 0;

  bool mouseControls = true;
  bool togglePressed = false;
};


Client client;

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
  
  client.mouse(window, xpos, ypos);
}



int Client::initialize()
{
  //make camera
  cameraMatrix = camera.getViewMatrix();
  projectionMatrix = glm::perspective(glm::radians(45.0f), 1024.0f / 768.0f, .1f, 100.0f);

  //make lights
  GenerateLights(8);

  //initialize glfw
  if (!glfwInit())
  {
    std::cout << "Could not initialize glfw.\n";
    return -1;
  }

  //make a window
  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(1024, 768, "Deferred Renderer", NULL, NULL);
  if (window == NULL)
  {
    std::cout << "Faled to open window\n";
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  //initialize glew
  glewExperimental = true;
  if (glewInit() != GLEW_OK)
  {
    std::cout << "Could not initialize glew.\n";
    glfwTerminate();
    return NULL;
  }

  currentTime = glfwGetTime();

  glEnable(GL_DEPTH_TEST);

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);


  //load in objects
  centerObject.loadOBJ("../models/teapot_mid_poly.obj");
  sphere.loadOBJ("../models/sphere_mid_poly.obj");

  dRenderer.Initialize();

  ImGui::CreateContext();
  ImGui::StyleColorsDark();

  ImGuiIO& io = ImGui::GetIO();

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init();

  return 0;
}

int Client::update()
{
  //update time
  float time = glfwGetTime();
  float dt = time - currentTime;
  currentTime = time;


  //input and camera updates
  ProcessInput();
  cameraMatrix = camera.getViewMatrix();


  //rotate lights
  glm::mat4 lightRotation = glm::rotate(glm::identity<glm::mat4>(), dt, glm::vec3(0.0f, 1.0f, 0.0f));
  for (int i = 0; i < lights.lightPositions.size(); ++i)
  {
    lights.lightPositions[i] = lightRotation * lights.lightPositions[i];
  }


  //draw calls happen here
  dRenderer.DrawStart(projectionMatrix * cameraMatrix);
  dRenderer.DrawGeometry(centerObject, glm::identity<glm::mat4>());
  dRenderer.DrawLighting(lights, camera.position, mode);
  if(copy)
    dRenderer.CopyDepth();

  for (int i = 0; i < lights.lightPositions.size(); ++i)
  {
    glm::mat4 transform = glm::translate(glm::identity<glm::mat4>(), glm::vec3(lights.lightPositions[i])) * glm::scale(glm::mat4(1.0), glm::vec3(.1, .1, .1));
    dRenderer.DrawForward(sphere, transform, lights.lightColors[i]);
  }


  glfwPollEvents();
  if (glfwWindowShouldClose(window) != 0)
    return -1;
  
  //imgui stuff
  ImGuiIO& io = ImGui::GetIO();

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  ImGui::Begin("Settings");
  ImGui::Text("Press F for mouse cursor");
  if (ImGui::Button("Toggle Depth Copying"))
  {
    copy = !copy;
  }
  ImGui::Text("Rendering modes:");
  if (ImGui::Button("Deffered Rendering"))
  {
    mode = 0;
  }
  if (ImGui::Button("Position"))
  {
    mode = 1;
  }
  if (ImGui::Button("Normal"))
  {
    mode = 2;
  }
  if (ImGui::Button("Diffuse"))
  {
    mode = 3;
  }
  ImGui::End();
  ImGui::Render();
  int displayW, displayH;
  glfwGetFramebufferSize(window, &displayW, &displayH);
  glViewport(0, 0, displayW, displayH);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


  glfwSwapBuffers(window);

  return 0;
}

int Client::shutdown()
{
  dRenderer.Shutdown();
  return 0;
}

void Client::GenerateLights(int count)
{
  glm::vec4 colors[] = { glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) };

  lights.lightPositions.clear();
  lights.lightColors.clear();

  float degrees = 2.0f * glm::pi<float>() / (float)count;
  glm::mat4 rotation = glm::rotate(glm::identity<glm::mat4>(), degrees, glm::vec3(0.0f, 1.0f, 0.0f));
  glm::vec4 point(2.0f, 0.0f, 0.0f, 1.0f);
  int j = 0;
  for (int i = 0; i < count; ++i)
  {
    lights.push_back(point, colors[j]);

    point = rotation * point;

    ++j;
    if (j > 3)
      j = 0;
  }
}

void Client::ProcessInput()
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
  {
    glfwSetWindowShouldClose(window, true);
  }

  if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
  {
    if (togglePressed == false)
    {
      mouseControls = !mouseControls;
      if(mouseControls)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      else
      {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        firstMouse = true;
      }
    }

    togglePressed = true;
  }
  else
  {
    togglePressed = false;
  }

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
  {
    camera.forward(0.5f);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
  {
    camera.right(-0.5f);
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
  {
    camera.forward(-0.5f);
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
  {
    camera.right(0.5f);
  }
}

void Client::mouse(GLFWwindow * window, double xpos, double ypos)
{
  if (!mouseControls)
    return;
  if (firstMouse)
  {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  camera.MouseMovement(xpos - lastX, lastY - ypos);

  lastX = xpos;
  lastY = ypos;
}


int main(void)
{

  if (client.initialize() != 0)
  {
    return 0;
  }

  while(client.update() == 0) {}

  client.shutdown();
}

