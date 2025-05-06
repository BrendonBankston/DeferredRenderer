#include "ShaderProgram.h"

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

std::string shaderCode(const char * filename)
{
  std::ifstream in(filename);

  std::stringstream string;
  if (in.is_open())
  {
    string << in.rdbuf();
  }
  return string.str();
}


GLuint CreateProgram(const char * vertexShader, const char * fragmentShader)
{

  std::string vertex_shader_text = shaderCode(vertexShader);
  const char * vertShader = vertex_shader_text.c_str();

  GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShaderID, 1, &vertShader, NULL);
  glCompileShader(vertexShaderID);

  GLint result = GL_FALSE;
  int infoLogLength;
  glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &result);
  glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
  if (result == GL_FALSE)
  {
    std::cout << "Vertex shader" << vertexShader << " failed to compile.\n";
    std::vector<char> vertexShaderErrorMessage(infoLogLength + 1);
    glGetShaderInfoLog(vertexShaderID, infoLogLength, NULL, &vertexShaderErrorMessage[0]);
    printf("%s\n", &vertexShaderErrorMessage[0]);
    return NULL;
  }

  std::string fragment_shader_text = shaderCode(fragmentShader);
  const char *fragShader = fragment_shader_text.c_str();

  GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShaderID, 1, &fragShader, NULL);
  glCompileShader(fragmentShaderID);

  glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &result);
  glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
  if (result == GL_FALSE)
  {
    std::cout << "Fragment shader" << fragmentShader << " failed to compile.\n";
    std::vector<char> fragmentShaderErrorMessage(infoLogLength + 1);
    glGetShaderInfoLog(fragmentShaderID, infoLogLength, NULL, &fragmentShaderErrorMessage[0]);
    printf("%s\n", &fragmentShaderErrorMessage[0]);
    return NULL;
  }


  GLuint programID = glCreateProgram();
  glAttachShader(programID, vertexShaderID);
  glAttachShader(programID, fragmentShaderID);
  glLinkProgram(programID);

  glGetProgramiv(programID, GL_LINK_STATUS, &result);
  if (result == GL_FALSE)
  {
    std::cout << "Program failed to link.\n";
    return NULL;
  }

  //glDetachShader(programID, vertexShaderID);
  //glDetachShader(programID, fragmentShaderID);
  glDeleteShader(vertexShaderID);
  glDeleteShader(fragmentShaderID);

  return programID;
}
