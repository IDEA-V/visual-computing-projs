#version 430

uniform sampler2D tex;

in vec2 texCoords;

layout(location = 0) out vec4 fragColor;

void main() {
    // --------------------------------------------------------------------------------
    //  TODO: Something is missing here.
    // --------------------------------------------------------------------------------
    fragColor = texture(tex, texCoords);
}
