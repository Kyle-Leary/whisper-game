#version 330 core

layout (location = 0) out vec4 color;

in vec3 lightColor;
in vec2 fsTexCoord;

uniform sampler2D tex_sampler;

void main()
{
	vec4 texColor = texture(tex_sampler, fsTexCoord);
	color = texColor * vec4(lightColor, 1) + vec4(0, 0.5, 0.5, 1); 
}
