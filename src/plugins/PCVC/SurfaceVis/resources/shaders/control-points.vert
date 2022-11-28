#version 430

// --------------------------------------------------------------------------------
//  TODO: Complete this shader.
// --------------------------------------------------------------------------------

uniform mat4 projMx;
uniform mat4 viewMx;
uniform vec3 pickedIdCol;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color;

out vec3 pointColor;

void main() {
    gl_Position = projMx * viewMx * vec4(in_position, 1.0f);

    pointColor = vec3(0.3f, 0.3f, 1.0f);
}
