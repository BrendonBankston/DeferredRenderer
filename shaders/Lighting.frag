#version 430

uniform sampler2D gPositionMap;
uniform sampler2D gNormalMap;
uniform sampler2D gColorMap;

uniform vec4 lightPositions[8];
uniform vec4 lightColors[8];

uniform vec3 viewPos;

uniform int mode;

in vec2 TexCoord;

out vec4 FragColor;



void main()
{
  vec3 FragPos = texture(gPositionMap, TexCoord).xyz;
  vec3 Normal = texture(gNormalMap, TexCoord).xyz;
  vec3 Diffuse = texture(gColorMap, TexCoord).xyz;
  float Specular = texture(gColorMap, TexCoord).a;
  
  if(mode != 0)
  {
    if(mode == 1)
      FragColor = vec4(FragPos, 1.0);
    if(mode == 2)
      FragColor = vec4(Normal, 1.0);
    if(mode == 3)
      FragColor = vec4(Diffuse, 1.0);
  }
  else
  {
  Normal = normalize(Normal);
  
  vec3 lighting = Diffuse * 0.1;
  vec3 viewDir = normalize(viewPos - FragPos);
  for(int i = 0; i < 8; ++i)
  {
    vec3 lightDir = normalize(lightPositions[i].xyz - FragPos);
    vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * lightColors[i].xyz;
    
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
    vec3 specular = lightColors[i].xyz * spec * Specular;
    
    float dist = length(lightPositions[i].xyz - FragPos);
    float attenuation = min(1.0f / (1.0f + 0.3f * dist + 0.5f * dist * dist), 1.0f);
    
    lighting += attenuation * diffuse + attenuation * specular;
  }
  
  FragColor = vec4(lighting, 1.0);
  }
}