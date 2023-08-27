#vertex
#version 330

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoords; 

out vec2 TexCoords;

uniform mat4 u_model;
// projection matrix in the HUD is special, we aren't using the normal UBO like in most 3d world shaders.
uniform mat4 projection;

void main() {
    TexCoords = texCoords;
    gl_Position = projection * u_model * vec4(position.xy, -1.0, 1.0); 
}


#fragment
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
