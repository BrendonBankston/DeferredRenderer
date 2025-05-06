#version 430

layout (location = 0) out vec3 gPositionMap;
layout (location = 1) out vec3 gNormalMap;
layout (location = 2) out vec4 gColorMap;

in vec3 Normal0;
in vec3 ViewPos0;


void main()
{
  gPositionMap = ViewPos0;
  gNormalMap = normalize(Normal0);
  gColorMap = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}