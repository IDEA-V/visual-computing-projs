#version 430

// --------------------------------------------------------------------------------
//  TODO: Complete this shader!
// --------------------------------------------------------------------------------

uniform int pickId;
uniform sampler2D tex;

in vec3 normal;
in vec2 texCoords;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = texture(tex, texCoords);
}
