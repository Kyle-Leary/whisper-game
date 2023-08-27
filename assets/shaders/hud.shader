#vertex
#version 330

// actually using a different vertex layout! 
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoords; // should the vertices have color, or should it just be a uniform in the fs?

out vec2 oTexCoords;

uniform mat4 u_model;
uniform mat4 projection;

void main() {
    oTexCoords = aTexCoords;
    gl_Position = projection * u_model * vec4(aPosition, -1.0, 1.0); 
}


#fragment
#version 330

in vec2 oTexCoords;

layout (location = 0) out vec4 color;

uniform sampler2D tex_sampler;

void main() {
    // i think texture() returns a vec4?
    color = texture(tex_sampler, oTexCoords);
}
