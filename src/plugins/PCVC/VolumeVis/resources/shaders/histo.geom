#version 430

uniform mat4 orthoProjMx;
uniform float binStepHalf;

layout(points) in;
layout(triangle_strip, max_vertices=4) out;

void main() {
    vec4 v = gl_in[0].gl_Position;
    // --------------------------------------------------------------------------------
    //  TODO: Expand the single point v to a quad, representing the bin.
    // --------------------------------------------------------------------------------
}
