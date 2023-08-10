layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoords;
out vec2 TexCoords;
uniform mat4 model;
uniform mat4 projection;
uniform float u_time;
void main() {
    TexCoords = texCoords;
    gl_Position = projection * model * vec4(position, -1.0, 1.0);
}
