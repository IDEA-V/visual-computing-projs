#version 430

// --------------------------------------------------------------------------------
//  TODO: Complete this shader!
// --------------------------------------------------------------------------------

uniform mat4 projMx;
uniform mat4 viewMx;
uniform mat4 modelMx;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_texCoords;

out vec3 normal;
out vec2 texCoords;

void main() {
    gl_Position = projMx * viewMx * modelMx * vec4(in_position, 1.0);
    normal = in_position;
    texCoords = in_texCoords;
}
