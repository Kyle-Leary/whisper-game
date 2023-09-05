#vertex
#version 330

// actually using a different vertex layout! 
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoords; // should the vertices have color, or should it just be a uniform in the fs?

out vec3 Position;
out vec2 TexCoords;

uniform mat4 u_model;
uniform mat4 u_projection;

void main() {
    TexCoords = aTexCoords;
    // zindex is literally implemented with the zindex in opengl. who would've thought.
    gl_Position = u_projection * u_model * vec4(aPosition, -90.0, 1.0); 
    Position = gl_Position.xyz;
}


#fragment
#version 330

in vec2 TexCoords;
in vec3 Position;

layout (location = 0) out vec4 color;

uniform sampler2D u_tex_sampler;

void main() {
    // i think texture() returns a vec4?
    color = texture(u_tex_sampler, TexCoords) + vec4(0, 0, 0, 0.1);
}
