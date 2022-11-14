#version 430

// --------------------------------------------------------------------------------
//  TODO: Complete this shader!
// --------------------------------------------------------------------------------

uniform int pickId;
uniform sampler2D tex;

in vec3 normal;
in vec2 texCoords;

layout(location = 0) out vec4 fragColor0; // Color
layout(location = 1) out uint fragColor1; // ID
layout(location = 2) out vec4 fragColor2; // Normals

void main() {

    fragColor0 = texture(tex, texCoords);
    fragColor1 = 255;
    fragColor2 = vec4(normal, 1.0);
}
