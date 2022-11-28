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
    // v[1] = v[1]*0.9 + 0.1;
    gl_Position = orthoProjMx * vec4(v.x, v.y*0.9+0.1, 0.0, 1.0);;
    EmitVertex();
    gl_Position = orthoProjMx * vec4(v.x, 0.1, 0.0, 1.0);
    EmitVertex();
    gl_Position = orthoProjMx * vec4(v.x + binStepHalf, v.y*0.9+0.1, 0.0, 1.0);
    EmitVertex();
    gl_Position = orthoProjMx * vec4(v.x + binStepHalf, 0.1, 0.0, 1.0);
    EmitVertex();

    // gl_Position = orthoProjMx * vec4(0.0, 1.0, 0.0, 1.0);
    // EmitVertex();
    // gl_Position = orthoProjMx * vec4(0.0, 0.0, 0.0, 1.0);
    // EmitVertex();
    // gl_Position = orthoProjMx * vec4(1.0, 1.0, 0.0, 1.0);
    // EmitVertex();
    // gl_Position = orthoProjMx * vec4(1.0, 0.0, 0.0, 1.0);
    // EmitVertex();

    EndPrimitive(); 
}
