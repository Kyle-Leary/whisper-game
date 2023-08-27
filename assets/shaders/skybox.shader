#vertex
#version 330 core
layout (location = 0) in vec3 aPos;

#include "mats.glinc"

out vec3 TexCoord;

void main() {
    TexCoord = aPos;

    mat4 rotView = mat4(mat3(view)); 
    gl_Position = projection * rotView * vec4(aPos, 1.0);
}


#fragment
#version 330 core

layout (location = 0) out vec4 color;

in vec3 TexCoord;

// special cubemap sampler type, not just another 2d texture.
uniform samplerCube u_cube_tex;

void main()
{
	// just copy in the texture from the main slot, this is all the skybox does.
	// the texcoord is just the interpolated position here.
	color = texture(u_cube_tex, TexCoord); 
}
