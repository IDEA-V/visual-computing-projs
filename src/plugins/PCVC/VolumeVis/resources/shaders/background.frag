#version 430

#define M_PI 3.14159265358979323846

uniform float aspect;

in vec2 texCoords;

layout(location = 0) out vec4 fragColor;

void main() {
    // --------------------------------------------------------------------------------
    //  TODO: Draw background pattern.
    // --------------------------------------------------------------------------------
    // Set up Checkerboard as Color
    int num = 64;
    float integer;
    float chessboard = floor(texCoords.x * num) + floor(texCoords.y * num / aspect);
    chessboard = modf(chessboard * 0.5, integer);
    chessboard *= 2;
    if(chessboard == 0.0f) fragColor = vec4(0.8f, 0.8f, 0.8f, 1.0f);
    else fragColor = vec4(0.45f, 0.45f, 0.45f, 1.0f);
}
