#version 430

#define M_PI 3.14159265358979323846

#define FLT_MAX 3.402823466e+38
#define FLT_MIN 1.175494351e-38

uniform sampler3D volumeTex; //!< 3D texture handle
uniform sampler1D transferTex;

uniform mat4 invViewMx;     //!< inverse view matrix
uniform mat4 invViewProjMx; //!< inverse view-projection matrix

uniform vec3 volumeRes; //!< volume resolution
uniform vec3 volumeDim; //!< volume dimensions

uniform int viewMode; //<! rendering method: 0: line-of-sight, 1: mip, 2: isosurface, 3: volume
uniform bool showBox;
uniform bool useRandom;

uniform int maxSteps;   //!< maximum number of steps
uniform float stepSize; //!< step size
uniform float scale;    //!< scaling factor

uniform float isovalue; //!< value for iso surface

uniform vec3 ambient;  //!< ambient color
uniform vec3 diffuse;  //!< diffuse color
uniform vec3 specular; //!< specular color

uniform float k_amb;  //!< ambient factor
uniform float k_diff; //!< diffuse factor
uniform float k_spec; //!< specular factor
uniform float k_exp;  //!< specular exponent

in vec2 texCoords;

layout(location = 0) out vec4 fragColor;

struct Ray {
    vec3 o; // origin of the ray
    vec3 d; // direction of the ray
};

/**
 * Test for an intersection with the bounding box.
 * @param r             The ray to use for the intersection test
 * @param boxmin        The minimum point (corner) of the box
 * @param boxmax        The maximum point (corner) of the box
 * @param tnear[out]    The distance to the closest plane intersection
 * @param tfar[out]     The distance to the farthest plane intersection
 */
bool intersectBox(Ray r, vec3 boxmin, vec3 boxmax, out float tnear, out float tfar) {
    // --------------------------------------------------------------------------------
    //  TODO: Calculate intersection between ray and box.
    // --------------------------------------------------------------------------------
    return false;
}

/**
 * Test if the given position is near a box edge.
 * @param pos           The position to test against
 */
bool isBoxEdge(vec3 pos) {
    // --------------------------------------------------------------------------------
    //  TODO: Check if the position is near an edge of the volume cube.
    // --------------------------------------------------------------------------------
    return false;
}

/**
 * Map world coordinates to texture coordinates.
 * @param pos           The world coordinates to transform to texture coordinates
 */
vec3 mapTexCoords(vec3 pos) {
    // --------------------------------------------------------------------------------
    //  TODO: Map world pos to texture coordinates for 3D volume texture.
    // --------------------------------------------------------------------------------
    return vec3(0.0);
}

/**
 * Calculate normals based on the volume gradient.
 */
vec3 calcNormal(vec3 pos) {
    // --------------------------------------------------------------------------------
    //  TODO: Calculate normals based on volume gradient.
    // --------------------------------------------------------------------------------
    return vec3(0.0);
}

/**
 * Calculate the correct pixel color using the Blinn-Phong shading model.
 * @param n             The normal at this pixel
 * @param l             The direction vector towards the light
 * @param v             The direction vector towards the viewer
 */
vec3 blinnPhong(vec3 n, vec3 l, vec3 v) {
    vec3 color = vec3(0.0);
    // --------------------------------------------------------------------------------
    //  TODO: Calculate correct Blinn-Phong shading.
    // --------------------------------------------------------------------------------
    return color;
}

/**
 * Pseudorandom function, returns a float value between 0.0 and 1.0.
 * @param seed          Positive integer that acts as a seed
 */
float random(uint seed) {
    // wang hash
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return float(seed) / 4294967296.0;
}

/**
 * The main function of the shader.
 */
void main() {
    vec4 color = vec4(0.0, 0.0, 0.0, 1.0);
    // --------------------------------------------------------------------------------
    //  TODO: Set up the ray and box. Do the intersection test and draw the box.
    // --------------------------------------------------------------------------------

    // --------------------------------------------------------------------------------
    //  TODO: Draw the volume based on the current view mode.
    // --------------------------------------------------------------------------------
    switch (viewMode) {
        case 0: { // line-of-sight
            // --------------------------------------------------------------------------------
            //  TODO: Implement line of sight (LoS) rendering.
            // --------------------------------------------------------------------------------
            break;
        }
        case 1: { // maximum-intesity projection
            // --------------------------------------------------------------------------------
            //  TODO: Implement maximum intensity projection (MIP) rendering.
            // --------------------------------------------------------------------------------
            break;
        }
        case 2: { // isosurface
            // --------------------------------------------------------------------------------
            //  TODO: Implement isosurface rendering.
            // --------------------------------------------------------------------------------
            break;
        }
        case 3: { // volume visualization with transfer function
            // --------------------------------------------------------------------------------
            //  TODO: Implement volume rendering.
            // --------------------------------------------------------------------------------
            break;
        }
        default: {
            color = vec4(1.0, 0.0, 0.0, 1.0);
            break;
        }
    }

    // --------------------------------------------------------------------------------
    //  TODO: Draw the box lines behind the volume, if the volume is transparent.
    // --------------------------------------------------------------------------------

    fragColor = color;
}
