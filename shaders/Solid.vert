#version 430 core

uniform mat4  cameraMat;
uniform mat4  objectMat;

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 modelPosition;

void main()
{
  gl_Position = cameraMat * objectMat * vec4(modelPosition, 1.0f);
}

