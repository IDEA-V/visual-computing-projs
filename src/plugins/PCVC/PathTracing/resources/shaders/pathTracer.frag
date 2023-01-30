#version 430
#define PI 3.14159265
#define EPSILON 0.0001

int BOUNCE_NUMBER = 10;

in vec2 texCoords;

layout(location = 0) out vec4 fragColor0;
layout(location = 1) out uint fragColor1;
// layout(location = 1) out vec4 fragColor1;
layout(location = 2) out vec4 fragColor2;

uniform int showFBOAtt;
uniform bool showDebug;

uniform float zNear;
uniform float zFar;
uniform int wWidth;
uniform int wHeight;

uniform mat4 invViewMx;
uniform mat4 invViewProjMx;


uniform float sphereRadius;
uniform uint objectNumber;
uniform float rand;
uniform int frameNumber;
uniform sampler2D fboTexColor;

struct Object {
    int type;
    uint id;
    bool emitting;
    int material;
    float specular;
    float roughness;
    float metalness;
    vec3 pos;
    vec4 albedo;
    float radius;
    vec3 s1;
    vec3 s2;
};

layout(std430, binding = 0) buffer layoutObject {
    Object objectList[];
};

struct Ray {
    vec3 o; // origin of the ray
    vec3 d; // direction of the ray
};

vec2 randState;
float rand2D()
{
    //return random number and update the seed at the same time
    randState.x = fract(sin(dot(randState.xy, vec2(12.9898, 78.233))) * 43758.5453);
    randState.y = fract(sin(dot(randState.xy, vec2(12.9898, 78.233))) * 43758.5453);
    
    return randState.x;
}

vec3 random_cos_weighted_hemisphere_direction( const vec3 n) {
    vec2 r = vec2(rand2D(), rand2D());
	vec3  uu = normalize(cross(n, abs(n.y) > .5 ? vec3(1.,0.,0.) : vec3(0.,1.,0.)));
	vec3  vv = cross(uu, n);
	float ra = sqrt(r.y);
	float rx = ra*cos(6.28318530718*r.x); 
	float ry = ra*sin(6.28318530718*r.x);
	float rz = sqrt(clamp(1.-r.y, 0.01, 0.99));
	vec3  rr = vec3(rx*uu + ry*vv + rz*n);
    return normalize(rr);
}

//=============================================================================
float pow2(float x) { 
    return x*x;
}

float GTR2_aniso(float NdotH, float HdotX, float HdotY, float ax, float ay) {
    return 1. / (PI * ax*ay * pow2( pow2(HdotX/ax) + pow2(HdotY/ay) + NdotH*NdotH ));
}

float smithG_GGX_aniso(float NdotV, float VdotX, float VdotY, float ax, float ay) {
    return 1. / (NdotV + sqrt( pow2(VdotX*ax) + pow2(VdotY*ay) + pow2(NdotV) ));
}

float schlickWeight(float cosTheta) {
    float m = clamp(1. - cosTheta, 0., 1.);
    return (m * m) * (m * m) * m;
}

void createBasis(vec3 normal, out vec3 tangent, out vec3 binormal){
    if (abs(normal.x) > abs(normal.y)) {
        tangent = normalize(vec3(0., normal.z, -normal.y));
    }
    else {
        tangent = normalize(vec3(-normal.z, 0., normal.x));
    }
    
    binormal = cross(normal, tangent);
}

vec3 sphericalDirection(float sinTheta, float cosTheta, float sinPhi, float cosPhi) {
    return vec3(sinTheta * cosPhi, sinTheta * sinPhi, cosTheta);
}


bool sameHemiSphere(const in vec3 wo, const in vec3 wi, const in vec3 normal) {
    return dot(wo, normal) * dot(wi, normal) > 0.0;
}

float pdfCos(float theta) {
    return cos(theta)/PI;
}

float pdfMicrofacetAniso(const in vec3 wi, const in vec3 wo, const in vec3 X, const in vec3 Y, const vec3 normal, const float roughness) {
    if (!sameHemiSphere(wo, wi, normal)) return 0.;
    vec3 wh = normalize(wo + wi);
    
    float aspect = sqrt(1.-0.0*.9);
    float alphax = max(.001, pow2(roughness)/aspect);
    float alphay = max(.001, pow2(roughness)*aspect);
    
    float alphax2 = alphax * alphax;
    float alphay2 = alphax * alphay;

    float hDotX = dot(wh, X);
    float hDotY = dot(wh, Y);
    float NdotH = dot(normal, wh);
    
    float denom = hDotX * hDotX/alphax2 + hDotY * hDotY/alphay2 + NdotH * NdotH;
    if( denom == 0. ) return 0.;
    float pdfDistribution = NdotH /(PI * alphax * alphay * denom * denom);
    return pdfDistribution/(4. * dot(wo, wh));
}

void disneyMicrofacetAnisoSample(out vec3 wi, const in vec3 wo, const in vec3 X, const in vec3 Y, const in vec2 u, const in vec3 normal, const in float roughness) {
    float cosTheta = 0., phi = 0.;
    
    float aspect = sqrt(1. - 0.0*.9);
    float alphax = max(.001, pow2(roughness)/aspect);
    float alphay = max(.001, pow2(roughness)*aspect);
    
    phi = atan(alphay / alphax * tan(2. * PI * u[1] + .5 * PI));
    
    if (u[1] > .5f) phi += PI;
    float sinPhi = sin(phi), cosPhi = cos(phi);
    float alphax2 = alphax * alphax, alphay2 = alphay * alphay;
    float alpha2 = 1. / (cosPhi * cosPhi / alphax2 + sinPhi * sinPhi / alphay2);
    float tanTheta2 = alpha2 * u[0] / (1. - u[0]);
    cosTheta = 1. / sqrt(1. + tanTheta2);
    
    float sinTheta = sqrt(max(0., 1. - cosTheta * cosTheta));
    vec3 whLocal = sphericalDirection(sinTheta, cosTheta, sin(phi), cos(phi));
         
    vec3 wh = whLocal.x * X + whLocal.y * Y + whLocal.z * normal;
    
    if(!sameHemiSphere(wo, wh, normal)) {
       wh *= -1.;
    }
            
    wi = normalize(reflect(-wo, wh));
}

vec3 disneyDiffuse(const in float NdotL, const in float NdotV, const in float LdotH, vec3 baseColor, float roughness) {

    float FL = schlickWeight(NdotL), FV = schlickWeight(NdotV);
    
    float Fd90 = 0.5 + 2. * LdotH*LdotH * roughness;
    float Fd = mix(1.0, Fd90, FL) * mix(1.0, Fd90, FV);
    
    return (1./PI) * Fd * baseColor;
}

vec3 disneyMicrofacetAnisotropic(float NdotL, float NdotV, float NdotH, float LdotH,
                                 const in vec3 L, const in vec3 V,
                                 const in vec3 H, const in vec3 X, const in vec3 Y,
                                 vec3 baseColor, float specular, float roughness, float metallic) {
    
    vec3 Cspec0 = mix(1.0 *.08*vec3(1.0), baseColor, metallic);
    
    float aspect = sqrt(1.-0.0*.9);
    float ax = max(.001, pow2(roughness)/aspect);
    float ay = max(.001, pow2(roughness)*aspect);
    float Ds = GTR2_aniso(NdotH, dot(H, X), dot(H, Y), ax, ay);

    float FH = schlickWeight(LdotH);
    vec3 Fs = mix(Cspec0, vec3(1), FH);
    float Gs;
    Gs  = smithG_GGX_aniso(NdotL, dot(L, X), dot(L, Y), ax, ay);
    Gs *= smithG_GGX_aniso(NdotV, dot(V, X), dot(V, Y), ax, ay);
    
    return Gs*Fs*Ds;
}

// Evaluate combined diffuse and specular BRDF
vec3 evalBRDF(vec3 n, vec3 v, vec3 l, vec3 albedo, float specular, float roughness, float metalness)
{
    // Common dot products
    float NoV = clamp(dot(n, v), 0.01, 0.99);
    float NoL = clamp(dot(n, l), 0.01, 0.99);
    vec3 h = normalize(v + l);
    float NoH = clamp(dot(n, h), 0.01, 0.99);
    float VoH = clamp(dot(v, h), 0.01, 0.99);
    float LoH = clamp(dot(l, h), 0.01, 0.99);

    vec3 diffuse = disneyDiffuse(NoL, NoV, clamp(dot(l, h), 0.01, 0.99), albedo, 0.1);
    diffuse = albedo/PI;

    vec3 tangent = cross(n, vec3(1.,0.,1.));
    vec3 binormal = normalize(cross(n, tangent));
    tangent = normalize(cross(n,binormal));

    return diffuse*(1-metalness) + disneyMicrofacetAnisotropic(NoL, NoV, NoH, LoH, l, v, h, tangent, binormal, albedo, specular, roughness, metalness);
}
//=============================================================================
float reflectance(float cosine, float ref_idx) {
    // Use Schlick's approximation for reflectance.
    float r0 = (1-ref_idx) / (1+ref_idx);
    r0 = r0*r0;
    return r0 + (1-r0)*pow((1 - cosine),5);
}

bool sampleLight(vec3 hitP, vec3 normal, inout float pdf, inout vec3 to_light){
    vec3 on_light = vec3(rand2D()*1.5-0.75, rand2D()*1.5-1.5, 3.4);
    to_light = on_light - hitP;
    float distance_squared = length(to_light)*length(to_light);
    to_light = normalize(to_light);

    if (dot(to_light, normal) < 0)
        return false;

    float light_area = 1.5*1.5;
    float light_cosine = abs(to_light.y);
    if (light_cosine < 0.000001)
        return false;

    pdf = distance_squared / (light_cosine * light_area);
    return true;
}

// bool flag1 = true;
// int count = 2;
vec4 brdf(vec3 hitD, vec3 hitP, inout vec3 normal, inout Ray r, Object o, out bool refracted) {
    // float seed = rand2D()+rand;
    refracted = false;
    r.o = hitP + normal*0.001;
    vec3 l = r.d;
    float pdf = 1.0;
    vec3 tangent = cross(normal, vec3(1.,0.,1.));
    vec3 binormal = normalize(cross(normal, tangent));
    tangent = normalize(cross(normal,binormal));
    
    switch (o.material) {
        //objects
        case 2:
            if (rand2D() > 0.5 && sampleLight(hitP, normal, pdf, r.d)) {
                //sample the light already
            }else{
                if (rand2D() > 0.5) {
                    //deffuse sampling
                    r.d = random_cos_weighted_hemisphere_direction(normal);
                }else{
                    //micro facet sampling
                    disneyMicrofacetAnisoSample(r.d, -l, tangent, binormal, vec2(rand2D(), rand2D()), normal, o.roughness);
                }
                pdf = 0.5 * pdfCos(dot(r.d, normal)) + 0.5 * pdfMicrofacetAniso(r.d, -l, tangent, binormal, normal, o.roughness);
            }

            pdf*=0.5;

            if( pdf < EPSILON ) return vec4(0.0,0.0,0.0,1.0);
            if (dot(r.d, normal) < 0) return vec4(0.0,0.0,0.0,1.0);

            return vec4(evalBRDF(normal, -l, r.d, o.albedo.rgb, o.specular, o.roughness, o.metalness)/pdf, 1.0);
            break;

        //background
        case 1:
            r.d = random_cos_weighted_hemisphere_direction(normal);
            pdf = pdfCos(dot(r.d, normal));


            if( pdf < EPSILON ) return vec4(0.0,0.0,0.0,1.0);
            if (dot(r.d, normal) < 0) return vec4(0.0,0.0,0.0,1.0);

            return vec4(o.albedo.rgb/PI/pdf, 1.0);
            break;
            
        //transparent objects
        case 3:
            // r.o = hitP + r.d*0.01;
            r.o = hitP;
            float ir = 2.0;

            vec3 refract_normal = normal;
            float refraction_ratio = 1/ir;
            bool flag = false;
            if (dot(r.d, normal) > 0) {
                refraction_ratio = ir;
                refract_normal = -normal;
                flag = true;
            }

            vec3 unit_direction = normalize(r.d);
            float cos_theta = min(dot(-unit_direction, refract_normal), 1.0);
            float sin_theta = sqrt(1.0 - cos_theta*cos_theta);

            bool cannot_refract = refraction_ratio * sin_theta > 1.0;
            vec3 direction;

            if (cannot_refract || reflectance(cos_theta, refraction_ratio) > rand2D()) {
                direction = reflect(unit_direction, refract_normal);
                //avoid negative color
                if (dot(direction, normal) < 0) normal = -normal;
            }else{
                direction = refract(unit_direction, refract_normal, refraction_ratio);
                refracted = true;
            }
            
            // if (!flag1 && count > 0) {
            //     direction = reflect(unit_direction, refract_normal);
            //     if (dot(direction, normal) < 0) normal = -normal;
            //     count -= 1;
            // }else{
            //     direction = refract(unit_direction, refract_normal, refraction_ratio);
            //     refracted = true;
            //     flag1 = false;
            // }

            r.d = normalize(direction);
            return vec4(1.0);
            break;
    }
}

bool intersectSphere(Ray r, float radius, vec3 pos, out float tNear, out float tFar, out vec3 normal) {
    vec3 o = r.o-pos;
    float a = dot(r.d, r.d);
    float b = 2.0*dot(o, r.d);
    float c = dot(o, o) - radius * radius;

    float discriminant = b*b-4*a*c;

    if (discriminant > 0) {
        tNear = (-b - sqrt(discriminant))/2/a;
        tFar = (-b + sqrt(discriminant))/2/a;

        //abandon too close intersection
        if (abs(tNear) < EPSILON) {
            tNear = tFar;
            vec3 hitPos = r.o + r.d*tFar;
            normal = normalize(hitPos-pos);
            return true;
        }
        if (tNear > 0) {
            //ray is shoot from somewhere very close to the surface
            vec3 hitPos = r.o + r.d*tNear;
            normal = normalize(hitPos-pos);
            return true;
        }
        else{
            //ray is shoot within the sphere
            if (length(r.o-pos)<radius) {
                tNear = tFar;
                vec3 hitPos = r.o + r.d*tFar;
                normal = normalize(hitPos-pos);
                return true;
            }
        }
    }

    return false;
}

bool intersectRect(Ray r, vec3 p, vec3 s1, vec3 s2 , out float tNear, out float tFar, out vec3 normal) {
    normal = normalize(cross(s1,s2));
    if (dot(r.d, normal) != 0) {
        tNear = dot((p-r.o), normal)/dot(r.d, normal);
        if (tNear>0) {
            vec3 hitPos = r.o + tNear*r.d;
            if (dot((hitPos-p), s1) >=0 && dot((hitPos-p), s1) <= length(s1)*length(s1) && dot((hitPos-p), s2) >=0 && dot((hitPos-p), s2) <= length(s2)*length(s2)) {
                return true;
            };

        }
    }

    return false;
}

bool intersectObject(Ray r, Object o, out float tNear, out float tFar, out vec3 normal) {
    switch (o.type) {
        case 0: //sphere
            return intersectSphere(r, o.radius, o.pos, tNear, tFar, normal);
            break;
        case 1: //rectangle
            if (intersectRect(r, o.pos, o.s1, o.s2, tNear, tFar, normal)){
                if (dot(r.d, normal) > 0) normal = -normal;
                return true;
            }else{
                return false;    
            }
            break;
    }

    return false;
}


float max3 (vec3 v) {
  return max (max (v.x, v.y), v.z);
}

float min3 (vec3 v) {
  return min (min (v.x, v.y), v.z);
}

vec3 zeroone (vec3 v) {
    vec3 a = v-min3(v); 
    return a/max3(a);
}

void rayColor(Ray ray, out vec4 fragColor0, out uint fragColor1, out vec4 fragColor2) {

    bool hit = true;
    int bounce = 0;
    vec4 emit = vec4(0.0,0.0,0.0,1.0);
    fragColor0 = vec4(1.0,1.0,1.0,1.0);
    vec4 debugColor;
    while (hit && bounce < BOUNCE_NUMBER) {
        hit=false;
        float tNear=0;
        float tFar=0;
        float current_tNear;
        float current_tFar;
        vec3 hitPos;
        vec3 normal;
        vec3 current_normal;
        vec4 color;
        uint material;
        Object hitO;

        for (int i = 0; i < objectNumber; i++) {
            Object o = objectList[i];
            if (intersectObject(ray, o, current_tNear, current_tFar, current_normal)) {
                if (tNear == 0 || current_tNear < tNear) {
                    hit = true;
                    tNear = current_tNear;
                    normal = current_normal;
                    hitO = o;

                    debugColor = vec4(ray.o, 1.0);
                    // fragColor1 = vec4(hitO.emitting);
                    // fragColor2 = vec4(ray.d, tNear);

                    if (bounce==0){
                        fragColor1 = o.id;
                        fragColor2 = vec4(normal/2+0.5, 1.0);
                    }
                }
            }
        }


        // fragColor2 = vec4(bounce, bounce, bounce, 1.0f);
        if (hit) {
            if (hitO.emitting) {
                //hit the light source
                hit = false;
                emit = hitO.albedo;
            }else{
                hitPos = ray.o + ray.d*tNear;
                bool refracted;
                vec4 color = brdf(ray.d, hitPos, normal, ray, hitO, refracted);

                if (!refracted){
                    fragColor0  *= color * dot(ray.d, normal);
                }else{
                    fragColor0  *= color;
                }
            }
        }

        bounce++;
    }
    fragColor0 = fragColor0*emit;
    if (showDebug) fragColor0 = debugColor;
}

void main() {
    randState = texCoords.xy+rand;

    float NDCPosX = 2.0f * (texCoords.x+ rand2D()/wWidth) - 1.0f;
    float NDCPosY = 2.0f * (texCoords.y+rand2D()/wHeight) - 1.0f;
    vec4 clipPos = vec4(NDCPosX, NDCPosY, -1.0, 1.0);

    struct Ray ray;
    ray.o = (invViewMx * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    vec4 viewPoint = invViewProjMx * clipPos;
    ray.d = normalize(viewPoint.xyz/viewPoint.w - ray.o);

    rayColor(ray, fragColor0, fragColor1, fragColor2);

    float gamma = 2.2;
    if (frameNumber >= 2) {
        vec3 lastColor = texture(fboTexColor, texCoords).rgb;
        //get back to linear space before blending
        lastColor = pow(lastColor, vec3(gamma));
        fragColor0 = vec4( lastColor*(frameNumber-1)/frameNumber + fragColor0.rgb/frameNumber, 1.0);
    }
    //gamma correction
    fragColor0.rgb = pow(fragColor0.rgb, vec3(1.0/gamma));
}
