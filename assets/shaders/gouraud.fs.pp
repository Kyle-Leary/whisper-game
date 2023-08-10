#version 330 core

layout (location = 0) out vec4 color;

// in gouraud, the frag shader is dumb. it just mirrors the passed in light.
in vec3 lightColor;
in vec2 fsTexCoord;

uniform sampler2D tex_sampler;

void main()
{
	// we need to sample the texture HERE, even in a vertex-bound gouraud shader method.
	// it just doesn't make sense to do in in the vs, unless you have an absurdly high poly count.
	vec4 texColor = texture(tex_sampler, fsTexCoord);
	color = texColor * vec4(lightColor, 1); 
}
