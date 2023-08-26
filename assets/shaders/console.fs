#version 330

in vec2 TexCoords;
in float XPos;

layout (location = 0) out vec4 color;

uniform sampler2D u_tex_sampler;

void main() {
	float scale = 0.3;
	float color_offset = scale * XPos;
	vec4 color_offset_vec = vec4(0.1 + color_offset, 0.1 + color_offset, 0.1, 0.1);
	color = (texture(u_tex_sampler, TexCoords) * 2) + color_offset_vec;
}
