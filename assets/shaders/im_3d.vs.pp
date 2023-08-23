#version 330 core

// just render positions with a specific color, render them in the normal perspective
// with the rest of the scene. the "model matrix" in this case will be baked into the
// immediate positions of the verts anyway.

layout (location = 0) in vec3 aPos;

out vec3 oPos;

layout(std140) uniform ViewProjection {
    mat4 view;
    mat4 projection;
};

void main() {
	oPos = aPos;
	gl_Position = projection * view * (vec4(aPos, 1.0));
}
