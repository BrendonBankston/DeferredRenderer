#pragma once
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "LightInfo.h"
#include "ObjectReader.h"

#include <vector>


class DeferredRenderer
{
public:
  int Initialize();
  void DrawStart(glm::mat4 cameraMatrix);
  void DrawGeometry(Object model, glm::mat4 modelMatrix);
  void DrawLighting(LightInfo lights, glm::vec4 cameraPos, int mode);
  void CopyDepth();
  void DrawForward(Object model, glm::mat4 modelMatrix, glm::vec4 modelColor);
  void Shutdown();

private:

  GLuint pGeometry;
  GLuint pLighting;
  GLuint pSolid;

  GLuint gBuffer;
  enum { gPosition, gNormal, gColor };
  GLuint textures[3];
  GLuint rboDepth;

  glm::mat4 camera;
  glm::mat4 transform; 
  glm::vec4 color;

  float quadVertices[12] = {
    -1.0f, 1.0f, 0.0f,
    -1.0f,-1.0f, 0.0f,
     1.0f, 1.0f, 0.0f,
     1.0f,-1.0f, 0.0f,
  };

  float quadUVs[8] = {
    0.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
  };

  GLuint quadVAO;
  GLuint quadVBO[2];

};