#version 430

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 TexCoords;

out vec2 TexCoord;

void main()
{
  TexCoord = TexCoords;
  gl_Position = vec4(Position, 1.0);
}