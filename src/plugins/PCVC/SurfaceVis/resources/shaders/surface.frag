#version 430

uniform bool showNormals;
uniform int freq;

layout(location = 0) out vec4 fragColor0;
layout(location = 1) out vec4 fragColor1;

in vec2 texCoords;
void main() {
    //fragColor0 = vec4(texCoords, 0.0f, 1.0f);

    // Set up Checkerboard as Color
    fragColor0 = vec4(texCoords, 0.0f, 1.0f);
    fragColor1 = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}
