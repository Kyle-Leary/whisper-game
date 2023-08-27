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
    mat4 inverse_binds[128];
    int num_bones;
};

// one material is rendered on one primitive at a time.
layout(std140) uniform MaterialBlock {
    vec4 albedo;
    float metallic;
    float roughness;
    vec3 emissive_factor;
    int double_sided;
};

// hardcode the slot numbers for now? i don't really see why i shouldn't.
#define BASE_COLOR_TEXTURE_SLOT 0
#define METALLIC_ROUGHNESS_TEXTURE_SLOT 1
#define NORMAL_TEXTURE_SLOT 2
#define OCCLUSION_TEXTURE_SLOT 3
#define EMISSIVE_TEXTURE_SLOT 4

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
