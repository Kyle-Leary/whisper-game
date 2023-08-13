#version 330 core

layout (location = 0) out vec4 color;

in vec2 oTexCoord;

uniform sampler2D tex_sampler;

void main()
{
	color = texture(tex_sampler, oTexCoord);
}
