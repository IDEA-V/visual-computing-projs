#version 430

// --------------------------------------------------------------------------------
//  TODO: Complete this shader!
// --------------------------------------------------------------------------------


in vec2 texCoords;

layout(location = 0) out vec4 fragColor;

uniform int showFBOAtt;
uniform sampler2D fboTexColor;
uniform usampler2D fboTexId;
uniform sampler2D fboTexNormals;
uniform sampler2D fboTexDepth;
uniform sampler2D lightFboTexColor;
uniform sampler2D lightFboTexDepth;

uniform float zNear;
uniform float zFar;

uniform mat4 CameraProjMx;
uniform mat4 CameraviewMx;
uniform mat4 lightProjMx;
uniform mat4 lightViewMx;

uniform vec3 lightDirection;

//true z value 
float trueZ(sampler2D depthMap, vec2 coord)
{
    float z_b = texture(depthMap, coord).r;
    float z_n = 2.0 * z_b - 1.0;
    float z = 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));

    return z;
}

//window coordinate to world coordinate
vec4 worldCoord(vec2 pos) {
    vec2 ndc = 2 * pos - 1.0;
    vec4 clip = vec4(ndc, 2*texture(fboTexDepth, pos).r-1, 1.0);
    vec4 viewSpacePosition = inverse(CameraProjMx) * clip;
    viewSpacePosition /= viewSpacePosition.w;
    vec4 worldSpacePosition = inverse(CameraviewMx) * viewSpacePosition;

    return worldSpacePosition;
}


void main() {
    float Ia = 1.0;
    float Iin = 1.0;
    float kamb = 0.2;
    float kdiff = 0.8;
    float kspec = 0.0;

    float Iamb = kamb * Ia;
    float Idiff = 0.0;
    float Ispec = 0.0;

    vec4 world;
    vec4 lightCoord;
    float lightDepth;

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
        float depth = trueZ(fboTexDepth, texCoords);
        depth = depth / (zFar-zNear);
        fragColor = vec4(depth);
        break;
    case 4:
        fragColor = texture(lightFboTexColor, texCoords);
        break;
    case 5:
        //depth from light source as light view 
        // depth = trueZ(lightFboTexDepth, texCoords);
        // depth = depth / (zFar-zNear);
        // fragColor = vec4(depth, depth, depth, 1.0);

        //depth from light source as camera view
        world = worldCoord(texCoords);
        lightCoord = lightProjMx * lightViewMx * world;
        lightCoord = vec4(lightCoord.xyz/lightCoord.w, 1.0);
        lightCoord = lightCoord/2 + 0.5;
        lightDepth = texture(lightFboTexDepth, vec2(lightCoord.x, lightCoord.y)).r;
        fragColor = vec4(lightDepth);
        break;
    case 6:
        vec4 color = texture(fboTexColor, texCoords);
        vec4 n = texture(fboTexNormals, texCoords);
        vec4 l = vec4(lightDirection, 0.0);
        float diffuse = kdiff * Iin * max(dot(n, l), 0.0);

        world = worldCoord(texCoords);
        lightCoord = lightProjMx * lightViewMx * world;
        lightCoord = vec4(lightCoord.xyz/lightCoord.w, 1.0);
        lightCoord = lightCoord/2 + 0.5;
        lightDepth = texture(lightFboTexDepth, vec2(lightCoord.x, lightCoord.y)).r;

        float lightDistance = lightCoord.z;

        float shadow = 0.0;
        if ( lightDepth < lightDistance && abs(lightDepth - lightDistance) > 0.000001) {
            shadow = 1.0;
        }
        
        fragColor = (Iamb + (1.0 - shadow) * (diffuse + Ispec)) * color;   
        break;
    }
}
