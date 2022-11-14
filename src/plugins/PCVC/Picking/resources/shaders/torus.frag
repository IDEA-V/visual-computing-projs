#version 430

// --------------------------------------------------------------------------------
//  TODO: Complete this shader!
// --------------------------------------------------------------------------------

uniform uint idColor;

in vec3 normal;
in vec2 texCoords;

layout(location = 0) out vec4 fragColor0; // Color
layout(location = 1) out uint fragColor1; // ID
layout(location = 2) out vec4 fragColor2; // Normals

void main() {
    // Set up Checkerboard as Color
    int num = 32;
    float integer;
    float chessboard = floor(texCoords.x * num) + floor(texCoords.y * num);
    chessboard = modf(chessboard * 0.5, integer);
    chessboard *= 2;
    if(chessboard == 0.0f) fragColor0 = vec4(0.5f, 0.5f, 0.5f, 1.0f);
    else fragColor0 = vec4(1.0f, 1.0f, 1.0f, 1.0f);

    fragColor1 = idColor;
    fragColor2 = vec4(normalize(normal), 0.0);
}
