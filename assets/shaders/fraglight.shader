#vertex
#version 330 core

// does the lighting distance calculations in the fragment shader rather than the vertex shader, 
// like a gouraud shader.

// use a typical basicvertex attribute set.
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

// the calculated gouraud output.
out vec3 fragPos; // get the rasterizer to interpolate the position for us.
out vec2 fsTexCoord;

uniform mat4 u_model;
#include "mats.glinc"

// all light discoloration is calculated on vertices and then interpolated by the rasterizer using the w coordinate.
// this is passed to the frag shader, who actually renders the coordinates.
void main() {
	fsTexCoord = aTexCoord;
	vec4 vert_full_pos = projection * view * u_model * (vec4(aPos, 1.0));
	fragPos = vec3(u_model * vec4(aPos, 1.0));
	gl_Position = vert_full_pos;
}


#fragment
#version 330 core

layout (location = 0) out vec4 color;

in vec3 fragPos;
in vec2 fsTexCoord;

#include "light.glinc"

uniform sampler2D tex_sampler;

vec3 apply_point_light(vec3 vert_pos, int light_index) {
	vec3 light_from = point_lights[light_index].position;
	vec4 pt_light_color = point_lights[light_index].color;
	float light_intensity = point_lights[light_index].intensity;
	float dist_from_light = distance(light_from, vert_pos);
	light_intensity /= pow(dist_from_light, 2.0);
	pt_light_color *= light_intensity;

	return pt_light_color.xyz;
}

vec3 apply_spot_light(vec3 vert_pos, int light_index) {
	return vec3(0);
}

vec3 apply_directional_light(vec3 vert_pos, int light_index) {
	return vec3(0);
}

void main()
{
	vec4 texColor = texture(tex_sampler, fsTexCoord);
	vec3 ambient = ambient_light.color.xyz;
	ambient *= ambient_light.intensity;
	vec3 diffuse = vec3(0);
	vec3 specular = vec3(0);

	// lighting generates diffuse and specular.
	// use the normal positions, not the transformed ones. lights are specified in object-space.
	for (int i = 0; i < n_point_lights; i++) {
		diffuse += apply_point_light(fragPos, i);
	}
	for (int i = 0; i < n_spot_lights; i++) {
		diffuse += apply_spot_light(fragPos, i);
	}
	for (int i = 0; i < n_directional_lights; i++) {
		diffuse += apply_directional_light(fragPos, i);
	}

	vec3 lightColor = vec3(0);
	lightColor += ambient;
	lightColor += diffuse;
	lightColor += specular;

	color = vec4(lightColor, 1.0) * texColor;
}
