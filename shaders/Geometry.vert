#version 430

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;

uniform mat4 objectMat;
uniform mat4 cameraMat;

out vec3 ViewPos0;
out vec3 Normal0;

void main()
{
  vec4 worldPos = objectMat * vec4(Position, 1.0);
  ViewPos0 = worldPos.xyz;
  
  Normal0 = transpose(inverse(mat3(objectMat))) * Normal;
  
  gl_Position = cameraMat * worldPos;
}