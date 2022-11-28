#version 430

uniform mat4 projMx;
uniform mat4 viewMx;

layout(location = 0) in vec3 in_position;

void main() {
    gl_Position = projMx * viewMx * vec4(in_position, 1.0f);
}
