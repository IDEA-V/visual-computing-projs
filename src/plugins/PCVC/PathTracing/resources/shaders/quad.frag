#version 430
in vec2 texCoords;

layout(location = 0) out vec4 fragColor;

uniform int showFBOAtt;
uniform sampler2D fboTexColor;
uniform usampler2D fboTexId;
uniform sampler2D fboTexNormals;
uniform sampler2D lightFboTexColor;

void main() {

    switch (showFBOAtt) {
    case 0:
        fragColor = texture(fboTexColor, texCoords);
        break;
    case 1:
        uint vec_tex = texture(fboTexId, texCoords).r;
        fragColor = vec4(float(vec_tex)/255, 0.0, 0.0, 1.0);
        break;
    case 2:
        fragColor = texture(fboTexNormals, texCoords);
        break;
    case 3:
        fragColor = texture(lightFboTexColor, texCoords);
        break;
    }
}
