#version 330 core

// just render the whole prim with the uniform input color.

layout (location = 0) out vec4 color;

in vec3 oPos;

// allow the uniform to also decide alpha.
uniform vec4 u_color;

void main() {
	color = u_color + (vec4(oPos, 1) / 200);
}
