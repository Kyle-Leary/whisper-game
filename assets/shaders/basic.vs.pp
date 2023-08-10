layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;
out vec3 vertexColor;
out vec2 TexCoord;
uniform mat4 model;
uniform mat4 view_rot;
uniform mat4 view_tf;
uniform mat4 projection;
uniform float u_time;
void main() {
 gl_Position = projection * view_rot * view_tf * model * (vec4(aPos, 1.0));
 TexCoord = aTexCoord;
 vertexColor = aColor;
}
