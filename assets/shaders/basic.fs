#version 330 core

layout (location = 0) out vec4 color;

uniform sampler2D main_slot;
in vec3 vertexNormal;
in vec2 TexCoord;

void main()
{
	color = texture(main_slot, TexCoord) * vec4(vertexNormal, 1); // this is COMPONENT-WISE, not a convolution.
}
