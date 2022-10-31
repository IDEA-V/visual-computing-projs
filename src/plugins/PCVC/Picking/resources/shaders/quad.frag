#version 430

// --------------------------------------------------------------------------------
//  TODO: Complete this shader!
// --------------------------------------------------------------------------------


in vec2 texCoords;

layout(location = 0) out vec4 fragColor;


void main() {
    vec4 color = vec4(texCoords, 0.0, 1.0);

    float Ia = 1.0;
    float Iin = 1.0;
    float kamb = 0.2;
    float kdiff = 0.8;
    float kspec = 0.0;

    float Iamb = kamb * Ia;
    float Idiff = 0.0;
    float Ispec = 0.0;

    fragColor = color;
}
