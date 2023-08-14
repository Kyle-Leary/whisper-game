#version 330

in vec2 TexCoords;

layout (location = 0) out vec4 color;

uniform sampler2D text_font_slot;
uniform vec3 text_base_color;

void main() {
    color = vec4(text_base_color, 1.0) * texture(text_font_slot, TexCoords);
}
