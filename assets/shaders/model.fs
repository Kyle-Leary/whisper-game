#version 330 core

layout (location = 0) out vec4 color;

in vec3 lightColor;
in vec2 oTexCoord;

uniform sampler2D tex_sampler;

void main()
{
	vec4 texColor = texture(tex_sampler, oTexCoord);
	color = texColor * vec4(lightColor, 1); 
}
