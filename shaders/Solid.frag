#version 430 core

uniform vec4 modelColor;

out vec3 color;

void main()
{
  color = modelColor.xyz;
}

