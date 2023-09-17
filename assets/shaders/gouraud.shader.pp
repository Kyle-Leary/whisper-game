#vertex
#version 330 core

// use a typical basicvertex attribute set.
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

// the calculated gouraud output.
out vec3 lightColor;
out vec2 fsTexCoord;

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
    float falloff;
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

// requires the light UBO to be also included.

// return a scaling modifier to the light, rather than modifying the light impurely.
vec3 apply_point_light(vec3 vert_pos, int light_index) {
	vec3 light_from = point_lights[light_index].position;
	vec4 pt_light_color = point_lights[light_index].color;
	float light_intensity = point_lights[light_index].intensity;
	float dist_from_light = distance(light_from, vert_pos);
	light_intensity /= dist_from_light;
	pt_light_color *= light_intensity;

	return pt_light_color.xyz;
}

vec3 apply_spot_light(vec3 vert_pos, int light_index) {
	return vec3(1);
}

vec3 apply_directional_light(vec3 vert_pos, int light_index) {
	return vec3(1);
}

// all light discoloration is calculated on vertices and then interpolated by the rasterizer using the w coordinate.
// this is passed to the frag shader, who actually renders the coordinates.
void main() {
	fsTexCoord = aTexCoord;

	vec3 ambient = ambient_light.color.xyz;
	ambient *= ambient_light.intensity;
	vec3 diffuse = vec3(0);
	vec3 specular = vec3(0);

	// lighting generates diffuse and specular.
	// use the normal positions, not the transformed ones. lights are specified in object-space.
	for (int i = 0; i < n_point_lights; i++) {
		diffuse += apply_point_light(aPos, i);
	}
	for (int i = 0; i < n_spot_lights; i++) {
		diffuse += apply_spot_light(aPos, i);
	}
	for (int i = 0; i < n_directional_lights; i++) {
		diffuse += apply_directional_light(aPos, i);
	}

	// setup the gouraud light color that will be passed to the fs.
	lightColor = vec3(0);
	lightColor += ambient;
	lightColor += diffuse;
	lightColor += specular;

	vec4 vert_full_pos = projection * view * u_model * (vec4(aPos, 1.0));
	gl_Position = vert_full_pos;
}


#fragment
#version 330 core

layout (location = 0) out vec4 color;

// in gouraud, the frag shader is dumb. it just mirrors the passed in light.
in vec3 lightColor;
in vec2 fsTexCoord;

uniform sampler2D tex_sampler;

void main()
{
	// we need to sample the texture HERE, even in a vertex-bound gouraud shader method.
	// it just doesn't make sense to do in in the vs, unless you have an absurdly high poly count.
	vec4 texColor = texture(tex_sampler, fsTexCoord);
	color = texColor * vec4(lightColor, 1); 
}
