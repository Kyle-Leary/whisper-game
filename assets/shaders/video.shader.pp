#vertex
#version 330 core

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


void main() {
	gl_Position = projection * view * u_model * (vec4(aPos, 1.0));
	TexCoord = aTexCoord;
	vertexNormal = aNormal;
}

#fragment
#version 330 core

layout (location = 0) out vec4 color;

uniform sampler2D base_color_texture;

in vec3 vertexNormal;
in vec2 TexCoord;

void main()
{
	color = texture(base_color_texture, TexCoord) * vec4(vertexNormal, 1); 
}
