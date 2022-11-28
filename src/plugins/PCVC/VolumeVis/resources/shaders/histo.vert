#version 430

uniform float maxBinValue;             //!< maximum value of all bins
uniform bool logPlot;                  //!< whether to use a log scale

layout(location = 0) in vec2 in_position;

void main() {
    // --------------------------------------------------------------------------------
    //  TODO: Set information for the histogram bar.
    // --------------------------------------------------------------------------------
    if (!logPlot) gl_Position = vec4(in_position.x, in_position.y / maxBinValue, 0.0, 1.0); // Linear scale
    else gl_Position = vec4(in_position.x, log(in_position.y) / log(maxBinValue), 0.0, 1.0); // Log scale
}
