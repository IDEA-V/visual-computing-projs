#version 430

uniform mat4 orthoProjMx;

layout(location = 0) in vec2 in_position;

out vec2 texCoords;

void main() {
    gl_Position = orthoProjMx * vec4(in_position, 0.0, 1.0);
    texCoords = in_position;
}
