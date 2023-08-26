#version 330

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoords; 

out vec2 TexCoords;

uniform mat4 u_model;
uniform mat4 u_projection;

void main() {
    TexCoords = aTexCoords;
    gl_Position = u_projection * u_model * vec4(aPosition, -1.0, 1.0); 
}
