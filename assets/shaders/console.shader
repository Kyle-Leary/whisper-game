#vertex
#version 330

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoords; 

out vec2 TexCoords;
out float XPos;

uniform mat4 u_model;
uniform mat4 u_projection;

void main() {
	TexCoords = aTexCoords;
	float z = -0.5;
	float x_factor = aPosition.x - 0.5; 
	float y_factor = aPosition.y - 0.5; 
	XPos = x_factor;
	float sine_offset = 1.3;
	float sine_amp = 0.9;
	z += sin(((x_factor + sine_offset) * 3.14159)) * sine_amp;  
	if (x_factor > 0.0) {
		z -= x_factor;
	} else {
		z += x_factor;
	}
	gl_Position = u_projection * u_model * vec4(aPosition.xy, z, 1.0); 
}


#fragment
#version 330

in vec2 TexCoords;
in float XPos;

layout (location = 0) out vec4 color;

uniform sampler2D u_tex_sampler;

void main() {
	vec4 tex_color = texture(u_tex_sampler, TexCoords);
	float scale = 0.3;
	float color_offset = scale * XPos;
	vec4 color_offset_vec = vec4(0.1 + color_offset, 0.1 + color_offset, 0.1, 0.1);
	if (tex_color.w < 0.5) {
		color_offset_vec.w += 0.5;
		color_offset_vec.x += 1;
	} 
	color = (tex_color * 2) + color_offset_vec;
}
