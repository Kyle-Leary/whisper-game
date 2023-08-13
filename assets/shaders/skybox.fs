#version 330 core

layout (location = 0) out vec4 color;

// special cubemap sampler type, not just another 2d texture.
uniform samplerCube u_cube_tex;

in vec3 TexCoord;

void main()
{
	// just copy in the texture from the main slot, this is all the skybox does.
	// the texcoord is just the interpolated position here.
	color = texture(u_cube_tex, TexCoord) + vec4(0.1, 0.1, 0.1, 0.5); 
}
