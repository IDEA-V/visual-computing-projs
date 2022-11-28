#version 430

// --------------------------------------------------------------------------------
//  TODO: Complete this shader.
// --------------------------------------------------------------------------------

in vec3 pointColor;

layout(location = 0) out vec4 fragColor0;

void main() {
    fragColor0 = vec4(pointColor, 1.0f);
}
