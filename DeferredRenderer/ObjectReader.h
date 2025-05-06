#pragma once

#include <stdlib.h>
#include <vector>
#include "GL/glew.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"


class Object
{
public:
  bool loadOBJ(const char *path);

  std::vector<glm::vec3> vertices;
  std::vector<unsigned int> faces;
  std::vector<glm::vec3> vertexNormals;
  std::vector<glm::vec3> faceNormals;

  std::vector<glm::vec3> vertexNormalLines;
  std::vector<glm::vec3> faceNormalLines;


  std::vector<glm::vec2> UVCylinderPosition;
  std::vector<glm::vec2> UVSpherePosition;
  std::vector<glm::vec2> UVCubePosition;

  std::vector<glm::vec2> UVCylinderNormal;
  std::vector<glm::vec2> UVSphereNormal;
  std::vector<glm::vec2> UVCubeNormal;

  glm::vec3 Ka;
  glm::vec3 Kd;
  glm::vec3 Ks;

  GLuint vertexArrayID;
  GLuint vertexBuffers[3];
};