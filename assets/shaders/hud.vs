#version 330

// actually using a different vertex layout! 
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoords; // should the vertices have color, or should it just be a uniform in the fs?

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 projection;
uniform float u_time;

void main() {
    TexCoords = texCoords;
    // use the projection to fit vertex-space to pixel-space on the gpu for the hud, so rendering positions are far more intuitive.
    // use the model for distortions? there isn't much of a point for this right now. we'll keep it, though.
    gl_Position = projection * model * vec4(position, -1.0, 1.0); // only x-y position? i might regret this later
}
