#version 330

// actually using a different vertex layout! 
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoords; // should the vertices have color, or should it just be a uniform in the fs?

out vec2 oTexCoords;

uniform mat4 model;
uniform mat4 projection;

void main() {
    oTexCoords = aTexCoords;
    gl_Position = projection * model * vec4(aPosition, -1.0, 1.0); 
}
