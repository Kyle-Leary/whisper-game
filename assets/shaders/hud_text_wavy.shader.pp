#vertex
#version 330

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoords; 

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 projection;

uniform float u_time;

void main() {
    TexCoords = texCoords;

    // modulate the yposition on a sine wave depending on how far along the current vertex is.
    float time_offset = sin(u_time * position.x * 50);
    time_offset /= 50;
    float y_pos = position.y + time_offset;

    gl_Position = projection * model * vec4(position.x, y_pos, -1.0, 1.0); 
}


#fragment
#version 330

in vec2 TexCoords;

layout (location = 0) out vec4 color;

uniform sampler2D text_font_slot;
uniform vec3 text_base_color;

void main() {
    color = (vec4(text_base_color, 1.0) * 2) * (texture(text_font_slot, TexCoords) + vec4(0.5, 0.5, 0.5, 0));
}
