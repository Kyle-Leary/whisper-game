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

uniform float u_time; // just overall useful to have a time parameter here. i don't think
// that we can just grab this directly through glsl?

void main() {
	gl_Position = projection * view * u_model * (vec4(aPos, 1.0));
	TexCoord = aTexCoord * (sin(u_time));
	vertexNormal = aNormal;
}

#fragment
#version 330 core

layout (location = 0) out vec4 color;

uniform sampler2D main_slot;
in vec3 vertexNormal;
in vec2 TexCoord;

void main()
{
	color = texture(main_slot, TexCoord) * vec4(vertexNormal, 1); // this is COMPONENT-WISE, not a convolution.
}
