#version 430

const vec2 vertex[4] = { 
    { -0.5f, -0.5f },
    {  0.5f, -0.5f },
    { -0.5f,  0.5f },
    { 0.5f,  0.5f },
};

void main() {
    gl_Position = vec4( vertex[gl_VertexID].x, vertex[gl_VertexID].y, 0.0, 1.0);
}
