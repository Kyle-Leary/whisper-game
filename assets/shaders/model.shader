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
#include "mats.glinc"
#include "bone.glinc"

#include "pbr_mat.glinc"

void main() {
	mat4 skin = mat4(0.0);

	for (int i = 0; i < 4; i++) {
	    float weight = aWeights[i];
	    int bone_index = aJoints[i];

	    mat4 bone_transform = bones[bone_index]; // current bone transformation
	    mat4 inverse_bind_matrix = inverse_binds[bone_index]; // inverse bind matrix for this bone

	    mat4 transformation = bone_transform * inverse_bind_matrix;
	    // linearly weight the full transformation in object space, then add that into the total skin modifier on this vertex.
	    skin += (transformation * weight);
	}

	gl_Position = skin * vec4(aPos, 1.0); // Apply the combined skinning transformation
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

uniform sampler2D tex_sampler;

void main()
{
	vec4 texColor = texture(tex_sampler, oTexCoord);
	color = texColor * vec4(lightColor, 1); 
}