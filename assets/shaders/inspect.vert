#version 330 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Color;
layout (location = 2) in vec2 a_UV;
layout (location = 3) in vec3 a_Normal;

out vec4 Color;
out vec2 UV;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;
uniform mat4 u_NormalMatrix;

void main()
{
    Color = vec4(a_Color, 1.0);
    UV = a_UV;
    Normal = vec3(normalize((u_NormalMatrix) * vec4(a_Normal,0.0)));
    FragPos = vec3(u_Model * vec4(a_Position, 1.0));
    gl_Position = u_Projection * u_View * vec4(FragPos, 1.0);

}