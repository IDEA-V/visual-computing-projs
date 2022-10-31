#version 430

// --------------------------------------------------------------------------------
//  TODO: Complete this shader!
// --------------------------------------------------------------------------------

uniform mat4 projMx;
uniform mat4 viewMx;
uniform mat4 modelMx;
uniform mat3 normalMx;

layout(points) in;
layout(triangle_strip, max_vertices=24) out;

out vec3 normal;
out vec2 texCoords;

void main() {
}
