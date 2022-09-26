#version 330 core

in vec4 Color;
in vec2 UV;
in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

uniform vec3 u_CameraPos;
uniform vec3 u_LightColor;
uniform vec3 u_ObjectColor;

void main()
{
   // ambient
   float ambientStrength = 0.1;
   vec3 ambient = ambientStrength * u_LightColor;

   // diffuse
   vec3 lightDir = normalize(u_CameraPos - FragPos);  
   vec3 diffuse = max(dot(normalize(Normal), lightDir), 0.0) * u_LightColor;
  
   vec3 result = (ambient + diffuse) * u_ObjectColor;
   FragColor = vec4(result, 1.0);
}