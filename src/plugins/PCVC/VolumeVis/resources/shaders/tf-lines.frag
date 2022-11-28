#version 430

uniform int channel; //!< which color channel to draw (R,G,B or A)

layout(location = 0) out vec4 fragColor;

void main() {
    vec4 color = vec4(0.0, 0.0, 0.0, 1.0);
    color[channel] = 1.0;
    // --------------------------------------------------------------------------------
    //  TODO: Set color of lines.
    // --------------------------------------------------------------------------------
    fragColor = color;
}
