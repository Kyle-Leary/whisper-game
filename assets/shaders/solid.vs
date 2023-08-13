#version 330 core

// a shader that renders a solid color.

// uses the typical layout.
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

// nothing out. the fs doesn't compute anything.
uniform mat4 model;
#include "mats.glinc"

#include "light.glinc"

void main() {
	gl_Position = projection * view * model * (vec4(aPos, 1.0));
}
