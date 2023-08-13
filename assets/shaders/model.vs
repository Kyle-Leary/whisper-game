#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
// extra attribute data for the per-vertex model data
// this is a direct index into the BoneData transforms.
layout (location = 3) in ivec4 aJoints;
layout (location = 4) in vec4 aWeights;

out vec2 oTexCoord;

uniform mat4 model;

#include "light.glinc"
#include "mats.glinc"
#include "bone.glinc"

void main() {
	mat4 skin = mat4(0.0);

	// sum over the four weights and joints provided for this vertex.
	for (int i = 0; i < 4; i++) {
		float weight = aWeights[i];
		int bone_index = aJoints[i];

		// take a weighted sum of all the joints and their local transforms into the skinning matrix.
		skin += weight * bones[bone_index];
	}

	// wtf why doesn't *= work??
	gl_Position = (vec4(aPos, 1.0));
	gl_Position = model * gl_Position;
	gl_Position = view * gl_Position;
	gl_Position = projection * gl_Position;

	// then, take the model from the base pose to the bone pose.
	// gl_Position *= skin;

	oTexCoord = aTexCoord;
}
