#version 330 core

// use a typical basicvertex attribute set.
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

// the calculated gouraud output.
out vec3 lightColor;
out vec2 fsTexCoord;

// define the basic light structures we'll need.
#include "light.glinc"

// just use one texture here.
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
	fsTexCoord = aTexCoord;

	// setup the gouraud light color that will be passed to the fs.
	lightColor = vec3(1, 1, 1);

	// ambient
	lightColor *= vec3(ambient_light.color.xyz);
	lightColor *= ambient_light.intensity;

	vec4 vert_full_pos = projection * view * model * (vec4(aPos, 1.0));
	vec3 vert_pos = vert_full_pos.xyz;

	// point_lights
	for (int i = 0; i < n_point_lights; i++) {
		vec3 light_from = point_lights[i].position;
		vec4 pt_light_color = point_lights[i].color;
		float light_intensity = point_lights[i].intensity;
		float dist_from_light = distance(light_from, vert_pos);
		light_intensity /= dist_from_light / 20;
		pt_light_color *= light_intensity;

		lightColor *= pt_light_color.xyz;
	}

	gl_Position = vert_full_pos;
}
