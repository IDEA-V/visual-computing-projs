#include "Objects.h"

#include <cmath>
#include <iostream>
#include <utility>
#include <vector>
#include <glm/gtx/string_cast.hpp>

#include "Picking.h"

using namespace OGL4Core2::Plugins::PCVC::Picking;

template<typename VertexDataType>
using VertexData = std::pair<std::vector<VertexDataType>, glowl::VertexLayout>;

static const double pi = std::acos(-1.0);

Object::Object(Picking& basePlugin, int id, std::shared_ptr<glowl::Texture2D> tex)
    : modelMx(glm::mat4(1.0f)),
      basePlugin(basePlugin),
      id(id),
      shaderProgram(nullptr),
      va(nullptr),
      tex(std::move(tex)) {}

/**
 * The object is rendered.
 * @param projMx   projection matrix
 * @param viewMx   view matrix
 */
void Object::draw(const glm::mat4& projMx, const glm::mat4& viewMx) {
    if (shaderProgram == nullptr || va == nullptr) {
        return;
    }
    // --------------------------------------------------------------------------------
    //  TODO Implement object rendering!
    // --------------------------------------------------------------------------------
    //use program and bind va
    shaderProgram->use();
    va->bindVertexArray();

    // Set matrix and pickIdCol as uniform
    shaderProgram->setUniform("projMx", projMx);
    shaderProgram->setUniform("viewMx", viewMx);
    shaderProgram->setUniform("modelMx", modelMx);
    shaderProgram->setUniform("idColor", (unsigned int)id);

    // Set texture as uniform
    if (tex != nullptr) {
        glActiveTexture(GL_TEXTURE0);
        tex->bindTexture();
        shaderProgram->setUniform("tex", 0);
    }
    
    va->draw();

    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

Base::Base(Picking& basePlugin, int id, std::shared_ptr<glowl::Texture2D> tex)
    : Object(basePlugin, id, std::move(tex)) {
    initShaders();

    const std::vector<float> baseVertices{
        // clang-format off
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,
        // clang-format on
    };

    const std::vector<float> baseNormals{
        // clang-format off
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        // clang-format on
    };

    const std::vector<float> baseTexCoords{
        // clang-format off
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
        // clang-format on
    };

    const std::vector<GLuint> baseIndices{0, 1, 2, 3};

    glowl::Mesh::VertexDataList<float> vertexDataBase{
        {baseVertices, {12, {{3, GL_FLOAT, GL_FALSE, 0}}}},
        {baseNormals, {12, {{3, GL_FLOAT, GL_FALSE, 0}}}},
        {baseTexCoords, {8, {{2, GL_FLOAT, GL_FALSE, 0}}}},
    };
    va = std::make_unique<glowl::Mesh>(vertexDataBase, baseIndices, GL_UNSIGNED_INT, GL_TRIANGLE_STRIP);
}

/**
 * This function is caled from the plugin after pressing 'R' to reload/recompile shaders.
 */
void Base::reloadShaders() {
    initShaders();
}

/**
 * Creates the shader program for this object.
 */
void Base::initShaders() {
    try {
        shaderProgram = std::make_unique<glowl::GLSLProgram>(glowl::GLSLProgram::ShaderSourceList{
            {glowl::GLSLProgram::ShaderType::Vertex, basePlugin.getStringResource("shaders/base.vert")},
            {glowl::GLSLProgram::ShaderType::Fragment, basePlugin.getStringResource("shaders/base.frag")}});
    } catch (glowl::GLSLProgramException& e) {
        std::cerr << e.what() << std::endl;
    }
}

/**
 * Constructs a new cube instance.
 * @param basePlugin A reference to your Picking plugin
 * @param id         Unique ID to use for the new cube instance
 * @param tex        Texture for the new cube instance
 */
Cube::Cube(Picking& basePlugin, int id, std::shared_ptr<glowl::Texture2D> tex)
    : Object(basePlugin, id, std::move(tex)) {
    initShaders();

    // --------------------------------------------------------------------------------
    //  TODO: Init cube vertex data!
    // --------------------------------------------------------------------------------
    // Cube geometry is constructed in geometry shader, therefore only a single point vertex is specified here.
    const std::vector<float> cubeVertices{0.0f, 0.0f, 0.0f};
    const std::vector<GLuint> cubeIndices{0};

    const std::vector<VertexData<float>> cubeVertex_data {
    {cubeVertices, {sizeof(float)*3, {{3, GL_FLOAT, GL_FALSE, 0}} } },
    };
    va = std::make_unique<glowl::Mesh>(cubeVertex_data, cubeIndices, GL_UNSIGNED_INT, GL_POINTS, GL_STATIC_DRAW);
}

void Cube::reloadShaders() {
    initShaders();
}

void Cube::initShaders() {
    // --------------------------------------------------------------------------------
    //  TODO: Init cube shader program!
    // --------------------------------------------------------------------------------
    try {
        shaderProgram = std::make_unique<glowl::GLSLProgram>(glowl::GLSLProgram::ShaderSourceList{
            {glowl::GLSLProgram::ShaderType::Vertex, basePlugin.getStringResource("shaders/cube.vert")},
            {glowl::GLSLProgram::ShaderType::Geometry, basePlugin.getStringResource("shaders/cube.geom")},
            {glowl::GLSLProgram::ShaderType::Fragment, basePlugin.getStringResource("shaders/cube.frag")} });
    }
    catch (glowl::GLSLProgramException& e) { std::cerr << e.what() << std::endl; }
}

/**
 * Constructs a new sphere instance.
 * @param basePlugin A reference to your Picking plugin
 * @param id         Unique ID to use for the new sphere instance
 * @param tex        Texture for the new sphere instance
 */
Sphere::Sphere(Picking& basePlugin, int id, std::shared_ptr<glowl::Texture2D> tex)
    : Object(basePlugin, id, std::move(tex)) {
    initShaders();

    // --------------------------------------------------------------------------------
    //  TODO: Init sphere vertex data.
    // --------------------------------------------------------------------------------
    const int resTheta = 128;
    const int resPhi = 64;
    const double radiusSphere = 0.5;

    std::vector<float> sphereVertices;
    std::vector<float> sphereTexCoords;
    std::vector<GLuint> sphereIndices;

    // Create sphere vertices and texture coordinates
    const float stepTheta = 2 * pi / float(resTheta);
    const float stepPhi = pi / float(resPhi);

    float currentSector = 0.0f;
    for (int i = 0; i <= resTheta; i++) {
        float currentStack = pi / 2.0;
        for (int j = 0; j <= resPhi; j++) {
            // Vertex
            sphereVertices.emplace_back( radiusSphere * cos(currentStack) * cos(currentSector) ); // vertex.x
            sphereVertices.emplace_back( radiusSphere * cos(currentStack) * sin(currentSector) ); // vertex.y
            sphereVertices.emplace_back( radiusSphere * sin(currentStack) ); // vertex.z
            // Texture coordinate
            sphereTexCoords.emplace_back( float(i) / resTheta ); // textcoord.u
            sphereTexCoords.emplace_back( float(j) / resPhi ); // textcoord.v
            currentStack -= stepPhi;
        }
        currentSector += stepTheta;
    }

    // Connect indices. Start from pole
    GLuint currentVertexOffset = 0;
    for (int i = 0; i <= resTheta; i++) {
        for (int j = 0; j <= resPhi; j++) {
            sphereIndices.emplace_back(currentVertexOffset);
            sphereIndices.emplace_back(currentVertexOffset + resPhi + 1);
            currentVertexOffset ++;
        }
    }

    std::vector<VertexData<float>> sphereVertex_data {
    {sphereVertices,  {sizeof(float)*3, {{3, GL_FLOAT, GL_FALSE, 0}} } },
    {sphereTexCoords, {sizeof(float)*2, {{2, GL_FLOAT, GL_FALSE, 0}} } }
    };

    va = std::make_unique<glowl::Mesh>(sphereVertex_data, sphereIndices, GL_UNSIGNED_INT, GL_TRIANGLE_STRIP, GL_STATIC_DRAW);
}

void Sphere::reloadShaders() {
    initShaders();
}

void Sphere::initShaders() {
    // --------------------------------------------------------------------------------
    //  TODO: Init sphere shader program!
    // --------------------------------------------------------------------------------
    try {
        shaderProgram = std::make_unique<glowl::GLSLProgram>(glowl::GLSLProgram::ShaderSourceList{
            {glowl::GLSLProgram::ShaderType::Vertex, basePlugin.getStringResource("shaders/sphere.vert")},
            {glowl::GLSLProgram::ShaderType::Fragment, basePlugin.getStringResource("shaders/sphere.frag")} });
    }
    catch (glowl::GLSLProgramException& e) { std::cerr << e.what() << std::endl; }
}

/**
 * Constructs a new torus instance.
 * @param basePlugin A reference to your Picking plugin
 * @param id         Unique ID to use for the new torus instance
 * @param tex        Texture for the new torus instance
 */
Torus::Torus(Picking& basePlugin, int id, std::shared_ptr<glowl::Texture2D> tex)
    : Object(basePlugin, id, std::move(tex)) {
    initShaders();

    // --------------------------------------------------------------------------------
    //  TODO: Init torus vertex data.
    // --------------------------------------------------------------------------------
    const int resT = 64; // Toroidal
    const int resP = 64; // Poloidal
    const float R = 0.34f;
    const float r = 0.16f;

    std::vector<float> torusVertices;
    std::vector<float> torusNormals;
    std::vector<float> torusTexCoords;
    std::vector<GLuint> torusIndices;

    const float stepT = 2*pi / float(resT);
    const float stepP = 2*pi / float(resP);
    float currentT = 0.0f;

    for (int i = 0; i <= resT; i++)
    {
        float sinT = sin(currentT);
        float cosT = cos(currentT);
        float currentP = 0.0f;
        for (int j = 0; j <= resP; j++)
        {
            float sinP = sin(currentP);
            float cosP = cos(currentP);
            // Vertex
            torusVertices.emplace_back( (R + r * cosP) * cosT ); // vertex.x
            torusVertices.emplace_back( (R + r * cosP) * sinT ); // vertex.y
            torusVertices.emplace_back( r * sinP); // vertex.z
            // Normal
            torusNormals.emplace_back( cosT * cosP ); // normal.x
            torusNormals.emplace_back( sinT * cosP ); // normal.y
            torusNormals.emplace_back( sinP ); // normal.z

            currentP += stepP;
        }
        currentT += stepT;   
    }

    //normalize to [0,1]
    const float textureStepT = 1.0 / float(resT);
    const float textureStepP = 1.0 / float(resP);
    float textureCurrentT = 0.0f;

    for (int i = 0; i <= resT; i++) {
        float textureCurrentP = 0.0f;
        for (int j = 0; j <= resP; j++) {
            // Texture
            torusTexCoords.emplace_back(textureCurrentP); // texture.u
            torusTexCoords.emplace_back(textureCurrentT); // texture.v
            textureCurrentP += textureStepP;
        }
        textureCurrentT += textureStepT;
    }

    // connect indices
    GLuint currentVertexOffset = 0;

    for (int i = 0; i < resT; i++) {
        for (int j = 0; j <= resP; j++) {
            torusIndices.emplace_back(currentVertexOffset);
            torusIndices.emplace_back(currentVertexOffset + resP + 1);
            currentVertexOffset++;
        }
    }

    std::vector<VertexData<float>> torusVertex_data {
    {torusVertices, {sizeof(float)*3, {{3, GL_FLOAT, GL_FALSE, 0}} } },
    {torusNormals, {sizeof(float)*3, {{3, GL_FLOAT, GL_FALSE, 0}} } },
    {torusTexCoords, {sizeof(float)*2, {{2, GL_FLOAT, GL_FALSE, 0}} } }
    };

    glowl::VertexLayout torusLayout{
        {0}, {{3, GL_FLOAT, GL_FALSE, 0}, {3, GL_FLOAT, GL_FALSE, 0}, {2, GL_FLOAT, GL_FALSE, 0}} };
    va = std::make_unique<glowl::Mesh>(torusVertex_data,
        torusIndices, GL_UNSIGNED_INT, GL_TRIANGLE_STRIP, GL_STATIC_DRAW);

}

void Torus::reloadShaders() {
    initShaders();
}

void Torus::initShaders() {
    // --------------------------------------------------------------------------------
    //  TODO: Init torus shader program!
    // --------------------------------------------------------------------------------
    try {
        shaderProgram = std::make_unique<glowl::GLSLProgram>(glowl::GLSLProgram::ShaderSourceList{
            {glowl::GLSLProgram::ShaderType::Vertex, basePlugin.getStringResource("shaders/torus.vert")},
            {glowl::GLSLProgram::ShaderType::Fragment, basePlugin.getStringResource("shaders/torus.frag")} });
    }
    catch (glowl::GLSLProgramException& e) { std::cerr << e.what() << std::endl; }
}
