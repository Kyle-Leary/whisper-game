#version 330

in vec2 TexCoords;

layout (location = 0) out vec4 color;

uniform sampler2D u_tex_sampler;

void main() {
    color = (texture(u_tex_sampler, TexCoords) * 2) + vec4(0.1, 0.1, 0.1, 0.1);
}
