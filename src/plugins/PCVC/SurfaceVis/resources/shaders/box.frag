#version 430

layout(location = 0) out vec4 fragColor0;
layout(location = 1) out vec4 fragColor1;

void main() {
    fragColor0 = vec4(1.0f);
    fragColor1 = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}
