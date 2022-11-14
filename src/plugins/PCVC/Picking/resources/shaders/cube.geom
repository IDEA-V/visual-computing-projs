#version 430

// --------------------------------------------------------------------------------
//  TODO: Complete this shader!
// --------------------------------------------------------------------------------

uniform mat4 projMx;
uniform mat4 viewMx;
uniform mat4 modelMx;

layout(points) in;
layout(triangle_strip, max_vertices=25) out;

out vec3 normal;
out vec2 texCoords;

vec4 transform(vec4 pos) {
    return projMx * viewMx * modelMx * pos;
}

void main() {
    vec4 origin = gl_in[0].gl_Position;

    // bottom
    normal = vec3(0.0, -1.0, 0.0);
    gl_Position = transform(origin + vec4(-0.5, -0.5, -0.5, 0.0)); // 0
    texCoords = vec2(0.25, 0.75);
    EmitVertex();
    gl_Position = transform(origin + vec4(0.5, -0.5, -0.5, 0.0)); // 3
    texCoords = vec2(0.5, 0.75);
    EmitVertex();
    gl_Position = transform(origin + vec4(-0.5, -0.5, 0.5, 0.0)); // 1
    texCoords = vec2(0.25, 0.5);
    EmitVertex();
    gl_Position = transform(origin + vec4(0.5, -0.5, 0.5, 0.0)); // 2
    texCoords = vec2(0.5, 0.5);
    EmitVertex();

    //front
    normal = vec3(0.0, 0.0, 1.0);
    gl_Position = transform(origin + vec4(-0.5, 0.5, 0.5, 0.0)); // 5
    texCoords = vec2(0.25, 0.25);
    EmitVertex();
    gl_Position = transform(origin + vec4(0.5, 0.5, 0.5, 0.0)); // 6
    texCoords = vec2(0.5, 0.25);
    EmitVertex();

    //top
    normal = vec3(0.0, 1.0, 0.0);
    gl_Position = transform(origin + vec4(-0.5, 0.5, -0.5, 0.0)); // 4
    texCoords = vec2(0.25, 0.0);
    EmitVertex();
    gl_Position = transform(origin + vec4(0.5, 0.5, -0.5, 0.0)); // 7
    texCoords = vec2(0.5, 0.0);
    EmitVertex();
    EndPrimitive();

    //left
    normal = vec3(-1.0, 0.0, 0.0);
    gl_Position = transform(origin + vec4(-0.5, -0.5, -0.5, 0.0)); // 9
    texCoords = vec2(0.0, 0.5);
    EmitVertex();
    gl_Position = transform(origin + vec4(-0.5, 0.5, -0.5, 0.0)); // 8
    texCoords = vec2(0.0, 0.25);
    EmitVertex();
    gl_Position = transform(origin + vec4(-0.5, -0.5, 0.5, 0.0)); // 1
    texCoords = vec2(0.25, 0.5);
    EmitVertex();
    gl_Position = transform(origin + vec4(-0.5, 0.5, 0.5, 0.0)); // 5
    texCoords = vec2(0.25, 0.25);
    EmitVertex();
    EndPrimitive();
    
    //right
    normal = vec3(1.0, 0.0, 0.0);
    gl_Position = transform(origin + vec4(0.5, 0.5, 0.5, 0.0)); // 6
    texCoords = vec2(0.5, 0.25);
    EmitVertex();
    gl_Position = transform(origin + vec4(0.5, -0.5, 0.5, 0.0)); // 2
    texCoords = vec2(0.5, 0.5);
    EmitVertex();
    gl_Position = transform(origin + vec4(0.5, 0.5, -0.5, 0.0)); // 10
    texCoords = vec2(0.75, 0.25);
    EmitVertex();
    gl_Position = transform(origin + vec4(0.5, -0.5, -0.5, 0.0)); // 11
    texCoords = vec2(0.75, 0.5);
    EmitVertex();

    //back
    normal = vec3(0.0, 0.0, -1.0);
    gl_Position = transform(origin + vec4(-0.5, 0.5, -0.5, 0.0)); // 12
    texCoords = vec2(1, 0.25);
    EmitVertex();
    gl_Position = transform(origin + vec4(-0.5, -0.5, -0.5, 0.0)); // 13
    texCoords = vec2(1, 0.5);
    EmitVertex();

    EndPrimitive();  
}

