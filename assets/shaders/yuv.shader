#vertex
#version 330 core

// shader that merges three seperate yuv textures in opengl together on the gpu-side.

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

uniform sampler2D y_tex;
uniform sampler2D u_tex;
uniform sampler2D v_tex;

in vec3 vertexNormal;
in vec2 TexCoord;

// we load one-channel textures as GL_RED for the format, since it mainly deals with RGB.
void main()
{
	vec2 tc = TexCoord;
	tc.y -= 0.5;
	tc.y *= -1;
	tc.y += 0.5;

	float y = texture(y_tex, tc).r;
	float u = texture(u_tex, tc).r;
	float v = texture(v_tex, tc).r;

	// Perform the YUV to RGB conversion
	float r = y + 1.402 * (v - 0.5);
	float g = y - 0.344136 * (u - 0.5) - 0.714136 * (v - 0.5);
	float b = y + 1.772 * (u - 0.5);

	color = vec4(r, g, b, 1.0);
}
