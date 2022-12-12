#version 430

// --------------------------------------------------------------------------------
//  TODO: Complete this shader.
// --------------------------------------------------------------------------------

in vec3 pointColor;
in vec3 idColor;

layout(location = 0) out vec4 fragColor0;
layout(location = 1) out vec4 fragColor1; // ID

void main() {
    fragColor0 = vec4(pointColor, 1.0f);
    fragColor1 = vec4(idColor, 1.0);

}
