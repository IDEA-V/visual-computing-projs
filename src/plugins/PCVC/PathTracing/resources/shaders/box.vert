#version 430

uniform float scale;
uniform mat4 projMx;
uniform mat4 viewMx;
uniform mat4 modelMx;

layout(location = 0) in vec3 in_position;

void main() {
    mat4 scaleMx = mat4(scale, 0, 0, 0, 0, scale, 0, 0, 0, 0, scale, 0, 0, 0, 0, 1);
    gl_Position = projMx * viewMx * modelMx * scaleMx * vec4(in_position, 1.0);
}
