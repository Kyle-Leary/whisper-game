#version 330

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoords; 

out vec2 TexCoords;

uniform mat4 model;
// projection matrix in the HUD is special, we aren't using the normal UBO like in most 3d world shaders.
uniform mat4 projection;

void main() {
    TexCoords = texCoords;
    gl_Position = projection * model * vec4(position.xy, -1.0, 1.0); 
}
