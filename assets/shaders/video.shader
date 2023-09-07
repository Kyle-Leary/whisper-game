#vertex
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 vertexNormal;
out vec2 TexCoord;

uniform mat4 u_model;
#include "mats.glinc"
#include "light.glinc"

void main() {
	gl_Position = projection * view * u_model * (vec4(aPos, 1.0));
	TexCoord = aTexCoord;
	vertexNormal = aNormal;
}

#fragment
#version 330 core

layout (location = 0) out vec4 color;

uniform sampler2D base_color_texture;

in vec3 vertexNormal;
in vec2 TexCoord;

void main()
{
	color = texture(base_color_texture, TexCoord) * vec4(vertexNormal, 1); 
}
