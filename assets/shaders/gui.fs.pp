#version 330

in vec2 TexCoords;
in vec3 Position;

layout (location = 0) out vec4 color;

uniform sampler2D u_tex_sampler;

void main() {
    // i think texture() returns a vec4?
    color = texture(u_tex_sampler, TexCoords) + vec4(0.4 * Position.z, 0.4 + (Position.z / 5), 0.4, 0.4);
}
