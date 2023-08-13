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

#define POINT_LIGHT_SLOTS 5
#define SPOT_LIGHT_SLOTS 5
#define DIRECTIONAL_LIGHT_SLOTS 5

struct SpotLight {
    vec3 position;
};

struct PointLight {
    vec3 position;
    vec4 color;
    float intensity;
    float range;
};

struct DirectionalLight {
    vec3 direction;
    vec4 color;
    float intensity;
};

struct AmbientLight {
    vec4 color;
    float intensity;
};

layout(std140) uniform LightData {
    SpotLight spot_lights[SPOT_LIGHT_SLOTS];
    int n_spot_lights;

    PointLight point_lights[POINT_LIGHT_SLOTS];
    int n_point_lights;

    DirectionalLight directional_lights[DIRECTIONAL_LIGHT_SLOTS];
    int n_directional_lights;

    AmbientLight ambient_light;
};

// return a scaling modifier to the light, rather than modifying the light impurely.
vec3 apply_point_light(vec3 vert_pos, int light_index) {
	vec3 light_from = point_lights[light_index].position;
	vec4 pt_light_color = point_lights[light_index].color;
	float light_intensity = point_lights[light_index].intensity;
	float dist_from_light = distance(light_from, vert_pos);
	light_intensity /= dist_from_light / 20;
	pt_light_color *= light_intensity;

	return pt_light_color.xyz;
}

vec3 apply_spot_light(vec3 vert_pos, int light_index) {
	return vec3(1);
}

vec3 apply_directional_light(vec3 vert_pos, int light_index) {
	return vec3(1);
}
layout(std140) uniform ViewProjection {
    mat4 view;
    mat4 projection;
};
layout(std140) uniform BoneData {
    // cheap out and only use half of the slots. our max block size isn't really that big, and the bone indices are usually represented as u8s anyway.
    mat4 bones[128];
    int num_bones;
};

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
