#version 430

// --------------------------------------------------------------------------------
//  TODO: Complete this shader!
// --------------------------------------------------------------------------------

layout (location = 0) in vec4 in_position;
layout (location = 1) in vec2 in_texture_coord;

// Output: Pass vertex color and texture coorinate to cube.frag
out vec4 position;
out vec2 textureCoord;

// Transform matrix
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main() {
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * in_position;
    position = in_position;
    textureCoord = in_texture_coord;
}
