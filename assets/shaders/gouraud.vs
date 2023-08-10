#version 330 core

// use a typical basicvertex attribute set.
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

// the calculated gouraud output.
out vec3 lightColor;
out vec2 fsTexCoord;

// just use one texture here.
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
	fsTexCoord = aTexCoord;
	gl_Position = projection * view * model * (vec4(aPos, 1.0));
}
