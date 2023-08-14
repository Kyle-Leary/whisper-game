#version 330

in vec2 oTexCoords;

layout (location = 0) out vec4 color;

uniform sampler2D tex_sampler;

void main() {
    // i think texture() returns a vec4?
    color = texture(tex_sampler, oTexCoords);
}
