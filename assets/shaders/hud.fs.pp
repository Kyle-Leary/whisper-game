in vec2 TexCoords;
layout (location = 0) out vec4 color;
uniform sampler2D ui_font_slot;
uniform vec3 ui_text_color;
void main() {
    color = vec4(ui_text_color, 1.0) * texture(ui_font_slot, TexCoords);
}
