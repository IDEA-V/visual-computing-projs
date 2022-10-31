#include "Picking.h"

#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <utility>

#include <imgui.h>

#include "Objects.h"
#include "core/Core.h"

using namespace OGL4Core2;
using namespace OGL4Core2::Plugins::PCVC::Picking;

/**
 * @brief Picking constructor.
 * Initlizes all variables with meaningful values and initializes
 * geometry, objects and shaders used in the scene.
 */
Picking::Picking(const Core::Core& c)
    : Core::RenderPlugin(c),
      wWidth(10),
      wHeight(10),
      aspect(1.0f),
      lastMouseX(0.0),
      lastMouseY(0.0),
      moveMode(ObjectMoveMode::None),
      projMx(glm::mat4(1.0)),
      fbo(0),
      fboTexColor(0),
      fboTexId(0),
      fboTexNormals(0),
      fboTexDepth(0),
      pickedObjNum(-1),
      lightZnear(0.1f),
      lightZfar(50.0f),
      lightProjMx(glm::mat4(1.0)),
      lightViewMx(glm::mat4(1.0)),
      lightFboWidth(2048),
      lightFboHeight(2048),
      lightFbo(0),
      lightFboTexColor(0),
      lightFboTexDepth(0),
      backgroundColor(glm::vec3(0.2f, 0.2f, 0.2f)),
      useWireframe(false),
      showFBOAtt(0),
      fovY(45.0),
      zNear(0.01f),
      zFar(20.0f),
      lightLong(0.0f),
      lightLat(90.00f),
      lightDist(10.0f),
      lightFoV(45.0f) {
    // Init Camera
    camera = std::make_shared<Core::OrbitCamera>(5.0f);
    core_.registerCamera(camera);

    // Initialize shaders and vertex arrays for quad and box.
    initShaders();
    initVAs();

    // --------------------------------------------------------------------------------
    //  TODO: Load textures from the "resources/textures" folder.
    //        Use the "getTextureResource" helper function.
    // --------------------------------------------------------------------------------

    // --------------------------------------------------------------------------------
    //  TODO: Setup the 3D scene. Add a dice, a sphere, and a torus.
    // --------------------------------------------------------------------------------
    std::shared_ptr<Object> o1 = std::make_shared<Base>(*this, 1, texBoard);
    o1->modelMx = glm::translate(o1->modelMx, glm::vec3(0.0f, 0.0f, -0.6f));
    o1->modelMx = glm::scale(o1->modelMx, glm::vec3(5.0f, 5.0f, 0.01f));
    objectList.emplace_back(o1);


    // Request some parameters
    GLint maxColAtt;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColAtt);
    std::cerr << "Maximum number of color attachments: " << maxColAtt << std::endl;

    GLint maxGeomOuputVerts;
    glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &maxGeomOuputVerts);
    std::cerr << "Maximum number of geometry output vertices: " << maxGeomOuputVerts << std::endl;

    // Initialize clear color and enable depth testing
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
}

/**
 * @brief Picking destructor.
 */
Picking::~Picking() {
    // --------------------------------------------------------------------------------
    //  Note: The glowl library uses the RAII principle. OpenGL objects are deleted in
    //        the destructor of the glowl wrapper objects. Therefore we must not delete
    //        them on our own. But keep this in mind and remember that this not always
    //        happens automatically.
    //
    //  TODO: Do not forget to clear all other allocated sources!
    // --------------------------------------------------------------------------------

    // Reset OpenGL state.
    glDisable(GL_DEPTH_TEST);
}

/**
 * @brief Render GUI.
 */
void Picking::renderGUI() {
    if (ImGui::CollapsingHeader("Picking", ImGuiTreeNodeFlags_DefaultOpen)) {
        camera->drawGUI();
        ImGui::ColorEdit3("Background Color", reinterpret_cast<float*>(&backgroundColor), ImGuiColorEditFlags_Float);
        ImGui::Checkbox("Wireframe", &useWireframe);
        // Dropdown menu to choose which framebuffer attachment to show
        ImGui::Combo("FBO attach.", &showFBOAtt, "Color\0IDs\0Normals\0Depth\0LightView\0LightDepth\0Deferred\0");

        // --------------------------------------------------------------------------------
        //  TODO: Setup ImGui elements for 'fovY', 'zNear', 'ZFar', 'lightLong', 'lightLat',
        //        'lightDist'. For the bonus task, you also need 'lightFoV'.
        // --------------------------------------------------------------------------------
    }
}

/**
 * @brief Picking render callback.
 */
void Picking::render() {
    renderGUI();

    // Update the matrices for current frame.
    updateMatrices();

    // --------------------------------------------------------------------------------
    //  TODO: First render pass to fill the FBOs. Call the the drawTo... method(s).
    // --------------------------------------------------------------------------------

    // --------------------------------------------------------------------------------
    //  TODO: In the second render pass, a window filling quad is drawn and the FBO
    //    textures are used for deferred shading.
    // --------------------------------------------------------------------------------
    glViewport(0, 0, wWidth, wHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 orthoMx = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f);

    shaderQuad->use();
    shaderQuad->setUniform("projMx", orthoMx);

    vaQuad->draw();

    glUseProgram(0); // release shaderQuad
}

/**
 * @brief Picking resize callback.
 * @param width  The current width of the window
 * @param height The current height of the window
 */
void Picking::resize(int width, int height) {

    if (width > 0 && height > 0) {
        wWidth = width;
        wHeight = height;
        aspect = static_cast<float>(wWidth) / static_cast<float>(wHeight);

        // Every time the window size changes, the size, of the fbo has to be adapted.
        deleteFBOs();
        initFBOs();
    }
}

/**
 * @brief Picking keyboard callback.
 * @param key      Which key caused the event
 * @param action   Which key action was performed
 * @param mods     Which modifiers were active (e. g. shift)
 */
void Picking::keyboard(Core::Key key, Core::KeyAction action, [[maybe_unused]] Core::Mods mods) {
    if (action != Core::KeyAction::Press) {
        return;
    }

    if (key == Core::Key::R) {
        std::cout << "Reload shaders!" << std::endl;
        initShaders();
        for (const auto& object : objectList) {
            object->reloadShaders();
        }
    } else if (key >= Core::Key::Key1 && key <= Core::Key::Key7) {
        showFBOAtt = static_cast<int>(key) - static_cast<int>(Core::Key::Key1);
    }
}

/**
 * @brief Picking mouse callback.
 * @param button   Which mouse button caused the event
 * @param action   Which mouse action was performed
 * @param mods     Which modifiers were active (e. g. shift)
 */
void Picking::mouseButton(Core::MouseButton button, Core::MouseButtonAction action, Core::Mods mods) {
    // --------------------------------------------------------------------------------
    //  TODO: Add mouse button functionality.
    // --------------------------------------------------------------------------------
}

/**
 * @brief Picking mouse move callback.
 * Called after the mouse was moved, coordinates are measured in screen coordinates but
 * relative to the top-left corner of the window.
 * @param xpos     The x position of the mouse cursor
 * @param ypos     The y position of the mouse cursor
 */
void Picking::mouseMove(double xpos, double ypos) {
    // --------------------------------------------------------------------------------
    //  TODO: Add mouse move functionality.
    // --------------------------------------------------------------------------------

    lastMouseX = xpos;
    lastMouseY = ypos;
}

/**
 * @brief Init shaders for the window filling quad and the box that is drawn around picked objects.
 */
void Picking::initShaders() {
    try {
        shaderQuad = std::make_unique<glowl::GLSLProgram>(glowl::GLSLProgram::ShaderSourceList{
            {glowl::GLSLProgram::ShaderType::Vertex, getStringResource("shaders/quad.vert")},
            {glowl::GLSLProgram::ShaderType::Fragment, getStringResource("shaders/quad.frag")}});
    } catch (glowl::GLSLProgramException& e) {
        std::cerr << e.what() << std::endl;
    }

    // --------------------------------------------------------------------------------
    //  TODO: Init box shader.
    // --------------------------------------------------------------------------------
}

/**
 * @brief Init vertex arrays.
 */
void Picking::initVAs() {
    //  Create a vertex array for the window filling quad.
    std::vector<float> quadVertices{
        // clang-format off
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f
        // clang-format on
    };
    std::vector<uint32_t> quadIndices{
        // clang-format off
        0, 1, 2,
        2, 1, 3
        // clang-format on
    };
    glowl::Mesh::VertexDataList<float> vertexDataQuad{{quadVertices, {8, {{2, GL_FLOAT, GL_FALSE, 0}}}}};
    vaQuad = std::make_unique<glowl::Mesh>(vertexDataQuad, quadIndices);

    // The box is used to indicate the selected object. It is made up of the eight corners of a unit cube that are
    // connected by lines.
    std::vector<float> boxVertices{
        // clang-format off
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        // clang-format on
    };

    std::vector<GLuint> boxEdges{
        // clang-format off
        0, 1,  0, 2,  1, 3,  2, 3,
        0, 4,  1, 5,  2, 6,  3, 7,
        4, 5,  4, 6,  5, 7,  6, 7
        // clang-format on
    };

    // --------------------------------------------------------------------------------
    //  TODO: Create the vertex arrays for box.
    // --------------------------------------------------------------------------------
}

/**
 * @brief Initialize all frame buffer objects (FBOs).
 */
void Picking::initFBOs() {
    if (wWidth <= 0 || wHeight <= 0) {
        return;
    }

    // --------------------------------------------------------------------------------
    //  TODO: Create a frame buffer object (FBO) for multiple render targets.
    //        Use the createFBOTexture method to initialize an empty texture.
    // --------------------------------------------------------------------------------

    // --------------------------------------------------------------------------------
    //  TODO (BONUS TASK): Create a frame buffer object for the view from the spot light.
    // --------------------------------------------------------------------------------
}

/**
 * @brief Delete all framebuffer objects.
 */
void Picking::deleteFBOs() {
    // --------------------------------------------------------------------------------
    //  TODO: Clean up all FBOs.
    // --------------------------------------------------------------------------------
}

/**
 * @brief Check status of bound framebuffer object (FBO).
 */
void Picking::checkFBOStatus() {
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    switch (status) {
        case GL_FRAMEBUFFER_COMPLETE: {
            std::cout << "FBO status: complete." << std::endl;
            break;
        }
        case GL_FRAMEBUFFER_UNDEFINED: {
            std::cerr << "FBO status: undefined." << std::endl;
            break;
        }
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: {
            std::cerr << "FBO status: incomplete attachment." << std::endl;
            break;
        }
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: {
            std::cerr << "FBO status: no buffers are attached to the FBO." << std::endl;
            break;
        }
        case GL_FRAMEBUFFER_UNSUPPORTED: {
            std::cerr << "FBO status: combination of internal buffer formats is not supported." << std::endl;
            break;
        }
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: {
            std::cerr << "FBO status: number of samples or the value for ... does not match." << std::endl;
            break;
        }
        default: {
            std::cerr << "FBO status: unknown framebuffer status error." << std::endl;
            break;
        }
    }
}

/**
 * @brief Create a texture for use in the framebuffer object.
 * @param width            Texture width
 * @param height           Texture height
 * @param internalFormat   Internal format of the texture
 * @param format           Format of the data: GL_RGB,...
 * @param type             Data type: GL_UNSIGNED_BYTE, GL_FLOAT,...
 * @param filter           Texture filter: GL_LINEAR or GL_NEAREST

 * @return texture handle
 */
GLuint Picking::createFBOTexture(int width, int height, const GLenum internalFormat, const GLenum format,
    const GLenum type, GLint filter) {
    // --------------------------------------------------------------------------------
    //  TODO: Generate an empty 2D texture. Set min/mag filters. Set wrap mode in (s,t).
    // --------------------------------------------------------------------------------
    return 0;
}

/**
 * @brief Update the matrices.
 */
void Picking::updateMatrices() {
    // --------------------------------------------------------------------------------
    //  TODO: Update the projection matrix (projMx).
    // --------------------------------------------------------------------------------

    // --------------------------------------------------------------------------------
    //  TODO: Update the light matrices (for bonus task only).
    // --------------------------------------------------------------------------------
}

/**
 * @brief Draw to framebuffer object.
 */
void Picking::drawToFBO() {
    if (!glIsFramebuffer(fbo)) {
        return;
    }

    // --------------------------------------------------------------------------------
    //  TODO: Render the scene to the FBO.
    // --------------------------------------------------------------------------------
}

/**
 * @brief Draw view of spot light into corresponding FBO.
 */
void Picking::drawToLightFBO() {
    if (!glIsFramebuffer(lightFbo)) {
        return;
    }

    // --------------------------------------------------------------------------------
    //  TODO: Render the scene from the view of the light (bonus task only).
    // --------------------------------------------------------------------------------
}

