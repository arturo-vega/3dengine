#version 460

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;
layout (location = 2) in float aHeight;

out vec2 TexCoord;


void main() {
	gl_Position = vec4(aPos, 1.0);
	TexCoord = aTex;
	vertexHeight = aHeight;
}