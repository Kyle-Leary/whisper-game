#version 330

in vec2 TexCoords;

layout (location = 0) out vec4 color;

uniform sampler2D text_font_slot;
uniform vec3 text_base_color;

void main() {
    vec4 tex_color = texture(text_font_slot, TexCoords);
    vec4 tc_offset = vec4(1, 1, 1, 0);
    tc_offset *= 2;
    tex_color += tc_offset;
    color = vec4(text_base_color, 1.0) * 2 * tex_color;
}
