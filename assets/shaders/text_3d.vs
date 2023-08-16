#version 330

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal; 
layout(location = 2) in vec2 a_texcoord; 

out vec2 f_texcoord;

uniform mat4 model;
#include "mats.glinc"
#include "light.glinc"

void main() {
	f_texcoord = a_texcoord;
	gl_Position = projection * view * model * vec4(a_position, 1.0); 
}
