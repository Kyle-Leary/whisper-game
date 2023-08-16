#version 330

in vec2 f_texcoord;

layout (location = 0) out vec4 color;

uniform sampler2D text_font_slot;
uniform vec3 text_base_color;

void main() {
    color = (texture(text_font_slot, f_texcoord));
}
