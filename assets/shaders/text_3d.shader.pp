#vertex
#version 330

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal; 
layout(location = 2) in vec2 a_texcoord; 

out vec2 f_texcoord;

uniform mat4 model;
layout(std140) uniform ViewProjection {
    mat4 view;
    mat4 projection;
};

void main() {
	f_texcoord = a_texcoord;
	gl_Position = projection * view * model * vec4(a_position, 1.0); 
}


#fragment
#version 330

in vec2 f_texcoord;

layout (location = 0) out vec4 color;

uniform sampler2D text_font_slot;
uniform vec3 text_base_color;

void main() {
    color = (texture(text_font_slot, f_texcoord) * vec4(text_base_color, 1));
}
