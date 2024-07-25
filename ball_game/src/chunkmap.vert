#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 FragPosition;
out vec2 TexCoord;
out vec3 Normal;
out float Height;
out float chunkHeight;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float mapHeight;

void main()
{
    FragPosition = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal; // inverse is not fast will want to do this by the cpu in future

    gl_Position = projection * view * vec4(FragPosition, 1.0);
    TexCoord = aTexCoord;
    Height = aPos.y;
    chunkHeight = mapHeight;
 }