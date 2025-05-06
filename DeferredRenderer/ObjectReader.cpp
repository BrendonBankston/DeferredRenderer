#include "ObjectReader.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

bool Object::loadOBJ(const char * path)
{

  std::ifstream file;
  file.open(path);
  if (!file.is_open())
  {
    std::cout << "failed to open file.\n";
    return false;
  }

  glm::vec4 greatestValues(0.0f, 0.0f, 0.0f, 1.0f);
  glm::vec4 lowestValues(0.0f, 0.0f, 0.0f, 1.0f);


  std::string line;
  while (std::getline(file, line))
  {
    std::stringstream stream(line);
    if (line[0] == 'v')
    {
      glm::vec3 vertex;
      char c;
      stream >> c;
      stream >> vertex.x;
      stream >> vertex.y;
      stream >> vertex.z;

      greatestValues.x = glm::max(vertex.x, greatestValues.x);
      greatestValues.y = glm::max(vertex.y, greatestValues.y);
      greatestValues.z = glm::max(vertex.z, greatestValues.z);
      lowestValues.x = glm::min(vertex.x, lowestValues.x);
      lowestValues.y = glm::min(vertex.y, lowestValues.y);
      lowestValues.z = glm::min(vertex.z, lowestValues.z);

      vertices.push_back(vertex);

    }
    else if (line[0] == 'f')
    {
      std::vector<unsigned int> indices;
      char c;
      stream >> c;
      while (1)
      {
        unsigned int index;
        stream >> index;
        indices.push_back(--index);
        if (!stream.good())
          break;
      }
      for (int i = 1; i < indices.size() - 1; ++i)
      {
        faces.push_back(indices[0]);
        faces.push_back(indices[i]);
        faces.push_back(indices[i + 1]);
      }
    }
  }
  file.close();

  glm::vec3 midpoints((greatestValues.x + lowestValues.x) / 2.0f, (greatestValues.y + lowestValues.y) / 2.0f, (greatestValues.z + lowestValues.z) / 2.0f);
  glm::mat4 translate = glm::translate(glm::identity<glm::mat4>(), -1.0f * midpoints);
  greatestValues = translate * greatestValues;
  lowestValues = translate * lowestValues;
  float scalar = 1.0f / glm::max(glm::max(greatestValues.x, greatestValues.y), greatestValues.z);
  glm::mat4 scale = glm::scale(glm::identity<glm::mat4>(), glm::vec3(scalar, scalar, scalar));
  greatestValues = scale * greatestValues;
  lowestValues = scale * lowestValues;

  for (int i = 0; i < vertices.size(); ++i)
  {
    glm::vec4 newVert = scale * translate * glm::vec4(vertices[i], 1.0f);
    vertices[i] = glm::vec3(newVert.x, newVert.y, newVert.z);


    float theta = glm::atan(newVert.z / newVert.x) / (2 * glm::pi<float>());

    UVCylinderPosition.push_back(glm::vec2(theta, newVert.y / 2.0f + 0.5f));

    float r = sqrt(newVert.x * newVert.x + newVert.y * newVert.y + newVert.z * newVert.z);
    float phi = glm::acos(newVert.y / r) / (2 * glm::pi<float>());
    UVSpherePosition.push_back(glm::vec2(theta, phi));

    float absX = fabs(newVert.x);
    float absY = fabs(newVert.y);
    float absZ = fabs(newVert.z);
    if (absX > absY && absX > absZ)
    {
      if (newVert.x >= 0)
        UVCubePosition.push_back(glm::vec2(-newVert.z, newVert.y));
      else
        UVCubePosition.push_back(glm::vec2(newVert.z, newVert.y));
    }
    else if (absY > absX && absY > absZ)
    {
      if (newVert.y >= 0)
        UVCubePosition.push_back(glm::vec2(newVert.x, -newVert.z));
      else
        UVCubePosition.push_back(glm::vec2(newVert.x, newVert.z));
    }
    else if (absZ > absX && absZ > absY)
    {
      if (newVert.z >= 0)
        UVCubePosition.push_back(glm::vec2(newVert.x, newVert.y));
      else
        UVCubePosition.push_back(glm::vec2(-newVert.x, newVert.y));
    }
  }

  std::vector < std::vector<glm::vec3>> adjacentNormals(vertices.size());
  //calculate face normals
  for (unsigned int i = 0; i < faces.size(); i += 3)
  {
    glm::vec3 side1 = vertices[faces[i + 1]] - vertices[faces[i]];
    glm::vec3 side2 = vertices[faces[i + 2]] - vertices[faces[i]];
    glm::vec3 normal = glm::normalize(glm::cross(side1, side2));
    faceNormals.push_back(normal);
    for (unsigned int j = i; j < i + 3; ++j)
    {
      bool unique = true;
      for (unsigned int k = 0; k < adjacentNormals[faces[j]].size(); ++k)
      {
        if (glm::distance(normal, adjacentNormals[faces[j]][k]) < 0.000001f)
        {
          unique = false;
        }
      }
      if (unique)
        adjacentNormals[faces[j]].push_back(normal);
    }
    //face normal lines

    glm::vec3 center = (1.0f / 3.0f) * (vertices[faces[i]] + vertices[faces[i + 1]] + vertices[faces[i + 2]]);
    faceNormalLines.push_back(center);
    faceNormalLines.push_back(center + .01f * glm::normalize(normal));
  }

  for (unsigned int i = 0; i < vertices.size(); ++i)
  {
    glm::vec3 vertexNormal(0, 0, 0);
    for (unsigned int j = 0; j < adjacentNormals[i].size(); ++j)
    {
      vertexNormal += adjacentNormals[i][j];
    }
    vertexNormals.push_back(glm::normalize(vertexNormal));

    vertexNormalLines.push_back(vertices[i]);
    vertexNormalLines.push_back(vertices[i] + .01f * glm::normalize(vertexNormal));

    float theta = glm::atan(vertexNormal.z / vertexNormal.x) / (2 * glm::pi<float>());

    UVCylinderNormal.push_back(glm::vec2(theta, vertexNormal.y / 2.0f + 0.5f));

    float r = sqrt(vertexNormal.x * vertexNormal.x + vertexNormal.y * vertexNormal.y + vertexNormal.z * vertexNormal.z);
    float phi = glm::acos(vertexNormal.y / r) / (2 * glm::pi<float>());
    UVSphereNormal.push_back(glm::vec2(theta, phi));

    float absX = fabs(vertexNormal.x);
    float absY = fabs(vertexNormal.y);
    float absZ = fabs(vertexNormal.z);
    if (absX > absY && absX > absZ)
    {
      if (vertexNormal.x >= 0)
        UVCubeNormal.push_back(glm::vec2(-vertexNormal.z, vertexNormal.y));
      else
        UVCubeNormal.push_back(glm::vec2(vertexNormal.z, vertexNormal.y));
    }
    else if (absY > absX && absY > absZ)
    {
      if (vertexNormal.y >= 0)
        UVCubeNormal.push_back(glm::vec2(vertexNormal.x, -vertexNormal.z));
      else
        UVCubeNormal.push_back(glm::vec2(vertexNormal.x, vertexNormal.z));
    }
    else if (absZ > absX && absZ > absY)
    {
      if (vertexNormal.z >= 0)
        UVCubeNormal.push_back(glm::vec2(vertexNormal.x, vertexNormal.y));
      else
        UVCubeNormal.push_back(glm::vec2(-vertexNormal.x, vertexNormal.y));
    }
  }


  glGenVertexArrays(1, &vertexArrayID);
  glBindVertexArray(vertexArrayID);

  glGenBuffers(3, vertexBuffers);

  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertexNormals.size(), &vertexNormals[0], GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBuffers[2]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * faces.size(), &faces[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  return true;
}
