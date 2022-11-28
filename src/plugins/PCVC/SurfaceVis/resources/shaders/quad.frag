#version 430

uniform sampler2D tex;

in vec2 texCoords;

layout(location = 0) out vec4 fragColor;

void main() {
    // --------------------------------------------------------------------------------
    //  TODO: Something is missing here.
    // --------------------------------------------------------------------------------
    fragColor = vec4(texCoords, 0.0f, 1.0f);
}
