#vertex
#version 330 core

// a shader that renders a solid color.

// uses the typical layout.
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

// nothing out. the fs doesn't compute anything.
uniform mat4 u_model;
#include "mats.glinc"

void main() {
	gl_Position = projection * view * u_model * (vec4(aPos, 1.0));
}


#fragment
#version 330 core

layout (location = 0) out vec4 color;

uniform vec4 u_render_color;

void main()
{
	color = vec4(u_render_color); 
}
