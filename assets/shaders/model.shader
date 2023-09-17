#vertex
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
// extra attribute data for the per-vertex model data
// this is a direct index into the BoneData transforms.
layout (location = 3) in ivec4 aJoints;
layout (location = 4) in vec4 aWeights;

out vec3 lightColor;
out vec2 oTexCoord;

uniform mat4 u_model;

#include "light.glinc"
#include "gouraud_helper.glinc"
#include "mats.glinc"
#include "bone.glinc"

#include "pbr_mat.glinc"

void main() {
	vec4 final_anim_pos = vec4(0);

	for (int i = 0; i < 4; i++) {
		float weight = aWeights[i];
		int bone_index = aJoints[i];

		mat4 bone_transform = bones[bone_index]; // current bone transformation
		vec4 animated_position = bone_transform * vec4(aPos, 1.0);

		final_anim_pos += (animated_position * weight);
	}

	// final_anim_pos = vec4(aPos, 1);

	gl_Position = final_anim_pos;
	gl_Position = u_model * gl_Position;
	gl_Position = view * gl_Position;
	gl_Position = projection * gl_Position;

	// setup the gouraud light color that will be passed to the fs.
	lightColor = vec3(1, 1, 1);

	// ambient
	lightColor *= vec3(ambient_light.color.xyz);
	lightColor *= ambient_light.intensity;

	vec3 vert_pos = gl_Position.xyz;

	for (int i = 0; i < n_point_lights; i++) {
		lightColor *= apply_point_light(vert_pos, i);
	}

	for (int i = 0; i < n_spot_lights; i++) {
		lightColor *= apply_spot_light(vert_pos, i);
	}

	for (int i = 0; i < n_directional_lights; i++) {
		lightColor *= apply_directional_light(vert_pos, i);
	}

	// lightColor *= albedo.xyz;

	oTexCoord = aTexCoord;
}

#fragment
#version 330 core

layout (location = 0) out vec4 color;

in vec3 lightColor;
in vec2 oTexCoord;

// pass in the baseColorTexture here, or some other equivalent.
// we'll assume that most of the models going into this shader are under some kind of PBR material.
uniform sampler2D tex_sampler;

void main()
{
	color = texture(tex_sampler, oTexCoord);
	color = color * vec4(lightColor, 1); 
}
