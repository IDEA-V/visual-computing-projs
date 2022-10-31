#include "Objects.h"

#include <cmath>
#include <iostream>
#include <utility>
#include <vector>

#include "Picking.h"

using namespace OGL4Core2::Plugins::PCVC::Picking;

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

}

void Cube::reloadShaders() {
    initShaders();
}

void Cube::initShaders() {
    // --------------------------------------------------------------------------------
    //  TODO: Init cube shader program!
    // --------------------------------------------------------------------------------
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

}

void Sphere::reloadShaders() {
    initShaders();
}

void Sphere::initShaders() {
    // --------------------------------------------------------------------------------
    //  TODO: Init sphere shader program!
    // --------------------------------------------------------------------------------
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
    const float radiusOut = 0.34f;
    const float radiusIn = 0.16f;

}

void Torus::reloadShaders() {
    initShaders();
}

void Torus::initShaders() {
    // --------------------------------------------------------------------------------
    //  TODO: Init torus shader program!
    // --------------------------------------------------------------------------------
}
