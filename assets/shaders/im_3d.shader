#vertex
#version 330 core

// just render positions with a specific color, render them in the normal perspective
// with the rest of the scene. the "model matrix" in this case will be baked into the
// immediate positions of the verts anyway.

layout (location = 0) in vec3 aPos;

#include "mats.glinc"

void main() {
	gl_Position = projection * view * (vec4(aPos, 1.0));
	gl_PointSize = 500.0;
}


#fragment
#version 330 core

// just render the whole prim with the uniform input color.

layout (location = 0) out vec4 color;

// allow the uniform to also decide alpha.
uniform vec4 u_color;

void main() {
	color = u_color;
}
