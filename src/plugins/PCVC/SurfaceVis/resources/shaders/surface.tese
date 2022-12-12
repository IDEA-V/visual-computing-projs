#version 430

//  Set the tessellation mode
layout(quads, equal_spacing, cw) in;

layout(std430, binding = 0) buffer NodeVecU {
readonly float UBuffer[];
};
layout(std430, binding = 1) buffer NodeVecV {
readonly float VBuffer[];
};
layout(std430, binding = 2) buffer NodeVecP {
readonly float P[];
};

uniform mat4 projMx;
uniform mat4 viewMx;
uniform int n;
uniform int m;

out vec2 texCoords;

float N0 (int uv, int i, float u) {
    float U[];
    if (uv == 0) {
        U = UBuffer;
    }else{
        U = VBuffer;
    }
    if (u >= U[i] && u < U[i+1]) return 1.0f;
    else return 0.0f;
}

float N1 (int uv, int i, float u) {
    float U[];
    if (uv == 0) {
        U = UBuffer;
    }else{
        U = VBuffer;
    }

    float quotient1;
    float quotient2; 
    if (U[i+3] - U[i] < 0.001) quotient1 = 0;
    else quotient1 = (u-U[i])/(U[i+3] - U[i]);
    if (U[i+3+1] - U[i+1] < 0.001) quotient2 = 0;
    else quotient2 = (U[i+3+1] - u)/(U[i+3+1] - U[i+1]);

    return quotient1*N0(uv, i,u) + quotient2*N0(uv, i+1, u);
}

float N2 (int uv, int i, float u) {
    float U[];
    if (uv == 0) {
        U = UBuffer;
    }else{
        U = VBuffer;
    }

    float quotient1;
    float quotient2; 
    if (U[i+3] - U[i] < 0.001) quotient1 = 0;
    else quotient1 = (u-U[i])/(U[i+3] - U[i]);
    if (U[i+3+1] - U[i+1] < 0.001) quotient2 = 0;
    else quotient2 = (U[i+3+1] - u)/(U[i+3+1] - U[i+1]);

    return quotient1*N1(uv, i,u) + quotient2*N1(uv, i+1, u);
}

float N3 (int uv, int i, float u) {
    float U[];
    if (uv == 0) {
        U = UBuffer;
    }else{
        U = VBuffer;
    }

    float quotient1;
    float quotient2; 
    if (U[i+3] - U[i] < 0.001) quotient1 = 0;
    else quotient1 = (u-U[i])/(U[i+3] - U[i]);
    if (U[i+3+1] - U[i+1] < 0.001) quotient2 = 0;
    else quotient2 = (U[i+3+1] - u)/(U[i+3+1] - U[i+1]);
    
    return quotient1*N2(uv, i,u) + quotient2*N2(uv, i+1, u);
}

void main() {
    // Interpolate the position of the vertex in world coord.
    vec4 p1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
    vec4 p2 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x);
    vec4 pos = mix(p1, p2, gl_TessCoord.y);

    vec3 S;
    for (int i = 0; i <= n; i++) {
        for (int j = 0; j <= m; j++) {
            int index = m * i + j;
            vec3 pij = vec3(P[3*index], P[3*index+1], P[3*index+2]);
            S += N3(0, i, gl_TessCoord.x)*N3(1, j, gl_TessCoord.y)*pij;
        }
    }
    vec4 outPos = vec4(S, 1.0f);
    gl_Position = projMx * viewMx * outPos;

    texCoords = gl_TessCoord.xy;
}
