#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

// in the skybox shader, the position is the texcoord.
out vec3 TexCoord;

uniform mat4 model;
#include "mats.glinc"

#include "light.glinc"

void main() {
	vec4 pos = projection * view * model * (vec4(aPos, 1.0));
	gl_Position = vec4(pos.x, pos.y, pos.w, pos.w);
	// 3d texcoords in the vs??? only for cubemaps??
	TexCoord = vec3(pos.x, pos.y, -pos.z);
}
