#vertex
#version 330 core

// shader that merges three seperate yuv textures in opengl together on the gpu-side.

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 vertexNormal;
out vec2 TexCoord;

uniform mat4 u_model;
layout(std140) uniform ViewProjection {
    mat4 view;
    mat4 projection;
};
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

void main() {
	gl_Position = projection * view * u_model * (vec4(aPos, 1.0));
	TexCoord = aTexCoord;
	vertexNormal = aNormal;
}

#fragment
#version 330 core

layout (location = 0) out vec4 color;

uniform sampler2D y_tex;
uniform sampler2D u_tex;
uniform sampler2D v_tex;

in vec3 vertexNormal;
in vec2 TexCoord;

// we load one-channel textures as GL_RED for the format, since it mainly deals with RGB.
void main()
{
	vec2 tc = TexCoord;
	tc.y -= 0.5;
	tc.y *= -1;
	tc.y += 0.5;

	float y = texture(y_tex, tc).r;
	float u = texture(u_tex, tc).r;
	float v = texture(v_tex, tc).r;

	// Perform the YUV to RGB conversion
	float r = y + 1.402 * (v - 0.5);
	float g = y - 0.344136 * (u - 0.5) - 0.714136 * (v - 0.5);
	float b = y + 1.772 * (u - 0.5);

	color = vec4(r, g, b, 1.0);
}
