#vertex
#version 330 core

// use a typical basicvertex attribute set.
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

// the calculated gouraud output.
out vec3 lightColor;
out vec2 fsTexCoord;

uniform mat4 u_model;
#include "mats.glinc"

#include "light.glinc"
#include "gouraud_helper.glinc"

// all light discoloration is calculated on vertices and then interpolated by the rasterizer using the w coordinate.
// this is passed to the frag shader, who actually renders the coordinates.
void main() {
	fsTexCoord = aTexCoord;

	vec3 ambient = ambient_light.color.xyz;
	ambient *= ambient_light.intensity;
	vec3 diffuse = vec3(0);
	vec3 specular = vec3(0);

	// lighting generates diffuse and specular.
	// use the normal positions, not the transformed ones. lights are specified in object-space.
	for (int i = 0; i < n_point_lights; i++) {
		diffuse += apply_point_light(aPos, i);
	}
	for (int i = 0; i < n_spot_lights; i++) {
		diffuse += apply_spot_light(aPos, i);
	}
	for (int i = 0; i < n_directional_lights; i++) {
		diffuse += apply_directional_light(aPos, i);
	}

	// setup the gouraud light color that will be passed to the fs.
	lightColor = vec3(0);
	lightColor += ambient;
	lightColor += diffuse;
	lightColor += specular;

	vec4 vert_full_pos = projection * view * u_model * (vec4(aPos, 1.0));
	gl_Position = vert_full_pos;
}


#fragment
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
