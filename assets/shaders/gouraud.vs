#version 330 core

// use a typical basicvertex attribute set.
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

// the calculated gouraud output.
out vec3 lightColor;
out vec2 fsTexCoord;

uniform mat4 model;
#include "mats.glinc"

#include "light.glinc"

// all light discoloration is calculated on vertices and then interpolated by the rasterizer using the w coordinate.
// this is passed to the frag shader, who actually renders the coordinates.
void main() {
	fsTexCoord = aTexCoord;

	// setup the gouraud light color that will be passed to the fs.
	lightColor = vec3(1, 1, 1);

	// ambient
	lightColor *= vec3(ambient_light.color.xyz);
	lightColor *= ambient_light.intensity;

	vec4 vert_full_pos = projection * view * model * (vec4(aPos, 1.0));
	vec3 vert_pos = vert_full_pos.xyz;

	for (int i = 0; i < n_point_lights; i++) {
		lightColor *= apply_point_light(vert_pos, i);
	}

	for (int i = 0; i < n_spot_lights; i++) {
		lightColor *= apply_spot_light(vert_pos, i);
	}

	for (int i = 0; i < n_directional_lights; i++) {
		lightColor *= apply_directional_light(vert_pos, i);
	}

	gl_Position = vert_full_pos;
}
