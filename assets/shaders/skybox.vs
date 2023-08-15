#version 330 core
layout (location = 0) in vec3 aPos;

#include "mats.glinc"
#include "light.glinc"

out vec3 TexCoord;

void main() {
    TexCoord = aPos;

    mat4 rotView = mat4(mat3(view)); 
    gl_Position = projection * rotView * vec4(aPos, 1.0);
}
