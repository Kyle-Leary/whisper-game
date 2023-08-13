#version 330 core

// use a typical basicvertex attribute set.
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

// the calculated gouraud output.
out vec3 lightColor;
out vec2 fsTexCoord;

// define the basic light structures we'll need.
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

// just use one texture here.
uniform mat4 model;
layout(std140) uniform ViewProjection {
    mat4 view;
    mat4 projection;
};

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

	// point_lights
	for (int i = 0; i < n_point_lights; i++) {
		apply_point_light(vert_pos, i);
	}

	for (int i = 0; i < n_spot_lights; i++) {
		apply_spot_light(vert_pos, i);
	}

	for (int i = 0; i < n_directional_lights; i++) {
		apply_directional_light(vert_pos, i);
	}

	gl_Position = vert_full_pos;
}
