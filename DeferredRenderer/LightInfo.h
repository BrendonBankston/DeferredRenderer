#pragma once
#include "glm/glm.hpp"
#include <vector>

struct LightInfo
{
  LightInfo() {};

  void push_back(glm::vec4 position, glm::vec4 color) { lightPositions.push_back(position); lightColors.push_back(color); }

  std::vector<glm::vec4> lightPositions;
  std::vector<glm::vec4> lightColors;
};

