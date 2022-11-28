#version 430

uniform mat4 orthoProjMx;
uniform int channel; //!< which channel to draw

layout(location = 0) in float in_position;
layout(location = 1) in vec4 in_values;

void main() {
    gl_Position = orthoProjMx * vec4(in_position.x, in_values[channel]*0.9+0.1, 0.0, 1.0);
}
