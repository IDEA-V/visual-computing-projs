#include "SurfaceVis.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>

#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <imgui_stdlib.h>

#include "core/Core.h"

const int IDX_OFFSET = 10;

using namespace OGL4Core2;
using namespace OGL4Core2::Plugins::PCVC::SurfaceVis;

/**
 * @brief SurfaceVis constructor.
 */
SurfaceVis::SurfaceVis(const Core::Core& c)
    : Core::RenderPlugin(c),
      wWidth(32),
      wHeight(32),
      lastMouseX(0.0),
      lastMouseY(0.0),
      projMx(glm::mat4(1.0f)),
      vaEmpty(0),
      maxTessGenLevel(0),
      degree_p(3), // can be assumed to be constant for this assignment
      degree_q(3), // can be assumed to be constant for this assignment
      // --------------------------------------------------------------------------------
      //  TODO: Initialize self defined variables here.
      // --------------------------------------------------------------------------------
      fovY(45.0f),
      showBox(false),
      showNormals(false),
      useWireframe(false),
      showControlPoints(1),
      pointSize(10.0f),
      dataFilename("test.txt"),
      // --------------------------------------------------------------------------------
      //  TODO: Initialize self defined GUI variables here.
      // --------------------------------------------------------------------------------
      ambientColor(glm::vec3(1.0f, 1.0f, 1.0f)),
      diffuseColor(glm::vec3(1.0f, 1.0f, 1.0f)),
      specularColor(glm::vec3(1.0f, 1.0f, 1.0f)),
      k_ambient(0.2f),
      k_diffuse(0.7f),
      k_specular(0.0f),
      k_exp(120.0f),
      freq(4) {
    // Init Camera
    camera = std::make_shared<Core::OrbitCamera>(2.0f);
    core_.registerCamera(camera);

    // --------------------------------------------------------------------------------
    //  TODO: Check and save maximum allowed tessellation level to 'maxTessGenLevel'
    // --------------------------------------------------------------------------------

    initShaders();
    initVAs();

    // --------------------------------------------------------------------------------
    //  TODO: Initialize a flat b-spline surface.
    // --------------------------------------------------------------------------------

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL); // Change depth function to overdraw on same depth for nicer control points.
}

/**
 * @brief SurfaceVis destructor.
 */
SurfaceVis::~SurfaceVis() {
    // --------------------------------------------------------------------------------
    //  TODO: Do not forget to clear all allocated resources.
    // --------------------------------------------------------------------------------

    glDisable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
}

/**
 * @brief GUI rendering.
 */
void SurfaceVis::renderGUI() {
    camera->drawGUI();

    ImGui::SliderFloat("FoVy", &fovY, 5.0f, 90.0f);
    ImGui::Checkbox("Show Box", &showBox);
    ImGui::Checkbox("Show Normals", &showNormals);
    ImGui::Checkbox("Wireframe", &useWireframe);
    ImGui::Combo("ShowCPoints", &showControlPoints, "no\0yes\0always\0");
    ImGui::SliderFloat("PointSize", &pointSize, 1.0f, 50.0f);
    ImGui::InputText("Filename", &dataFilename);
    if (ImGui::Button("Load File")) {
        loadControlPoints(dataFilename);
    }
    ImGui::SameLine();
    if (ImGui::Button("Save File")) {
        saveControlPoints(dataFilename);
    }

    // --------------------------------------------------------------------------------
    //  TODO: Draw GUI for all added GUI variables.
    // --------------------------------------------------------------------------------

    ImGui::ColorEdit3("Ambient", reinterpret_cast<float*>(&ambientColor), ImGuiColorEditFlags_Float);
    ImGui::ColorEdit3("Diffuse", reinterpret_cast<float*>(&diffuseColor), ImGuiColorEditFlags_Float);
    ImGui::ColorEdit3("Specular", reinterpret_cast<float*>(&specularColor), ImGuiColorEditFlags_Float);

    ImGui::SliderFloat("k_amb", &k_ambient, 0.0f, 1.0f);
    ImGui::SliderFloat("k_diff", &k_diffuse, 0.0f, 1.0f);
    ImGui::SliderFloat("k_spec", &k_specular, 0.0f, 1.0f);
    ImGui::SliderFloat("k_exp", &k_exp, 0.0f, 5000.0f);

    ImGui::InputInt("freq", &freq);
    freq = std::clamp(freq, 0, 100);
}

/**
 * @brief SurfaceVis render callback.
 */
void SurfaceVis::render() {
    renderGUI();

    // --------------------------------------------------------------------------------
    //  TODO: Draw to the fbo.
    // --------------------------------------------------------------------------------

    glViewport(0, 0, wWidth, wHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 orthoMx = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f);

    shaderQuad->use();
    shaderQuad->setUniform("projMx", orthoMx);

    // --------------------------------------------------------------------------------
    //  TODO: Draw the screen-filling quad using the previous fbo as input texture.
    // --------------------------------------------------------------------------------

    vaQuad->draw();

    // --------------------------------------------------------------------------------
    //  TODO: Clean up after rendering
    // --------------------------------------------------------------------------------

    glUseProgram(0);
}

/**
 * @brief SurfaceVis resize callback.
 * @param width
 * @param height
 */
void SurfaceVis::resize(int width, int height) {
    if (width > 0 && height > 0) {
        wWidth = width;
        wHeight = height;
        // --------------------------------------------------------------------------------
        //  TODO: Initilialize the FBO again with the new width and height
        // --------------------------------------------------------------------------------
    }
}

/**
 * @brief SurfaceVis keyboard callback.
 * @param key      Which key caused the event
 * @param action   Which key action was performed
 * @param mods     Which modifiers were active (e. g. shift)
 */
void SurfaceVis::keyboard(Core::Key key, Core::KeyAction action, [[maybe_unused]] Core::Mods mods) {

    if (action != Core::KeyAction::Press) {
        return;
    }

    // --------------------------------------------------------------------------------
    //  TODO: Add keyboard functionality, use the core enums such as Core::Key::R:
    //    - Press 'R': Reload shaders.
    //    - Press 'X': Deselect control point.
    //    - Press 'Right': Select next control point.
    //    - Press 'Left': Select previous control point.
    //    - Press 'L': Load control points file.
    //    - Press 'S': Save control points file.
    //    - Press 'Backspace': Reset control points.
    // --------------------------------------------------------------------------------
}

/**
 * @brief SurfaceVis mouse callback.
 * @param button   Which mouse button caused the event
 * @param action   Which mouse action was performed
 * @param mods     Which modifiers were active (e. g. shift)
 */
void SurfaceVis::mouseButton(Core::MouseButton button, Core::MouseButtonAction action, Core::Mods mods) {
    // --------------------------------------------------------------------------------
    //  TODO: Implement picking.
    // --------------------------------------------------------------------------------
}

/**
 * @brief SurfaceVis mouse move callback.
 * Called after the mouse was moved, coordinates are measured in screen coordinates but
 * relative to the top-left corner of the window.
 * @param xpos     The x position of the mouse cursor
 * @param ypos     The y position of the mouse cursor
 */
void SurfaceVis::mouseMove(double xpos, double ypos) {
    // --------------------------------------------------------------------------------
    //  TODO: Implement view corrected movement of the picked control point.
    //        - Ctrl + Right Mouse Button: move in xy-plane
    //        - Ctrl + Middle Mouse Button: move in z-direction
    //          (if you don't have a middle mouse button, you can also use another
    //           key in combination with Ctrl)
    // --------------------------------------------------------------------------------

    lastMouseX = xpos;
    lastMouseY = ypos;
}

/**
 * @brief Convert object id to unique color.
 * @param id       Object id
 * @return color
 */
glm::vec3 SurfaceVis::idToColor(unsigned int id) {
    glm::ivec3 color(0);
    color.r = id % 256;
    color.g = (id >> 8u) % 256;
    color.b = (id >> 16u) % 256;
    return glm::vec3(color) / 255.0f;
}

/**
 * @brief Convert color to object id.
 * @param buf     Color defined as 3-array [red, green, blue]
 * @return id
 */
unsigned int SurfaceVis::colorToId(const unsigned char col[3]) {
    auto b1 = static_cast<unsigned int>(col[0]);
    auto b2 = static_cast<unsigned int>(col[1]);
    auto b3 = static_cast<unsigned int>(col[2]);
    return (b3 << 16u) + (b2 << 8u) + b1;
}

/**
 * @brief Initialize shaders.
 */
void SurfaceVis::initShaders() {
    // Initialize shader for rendering fbo content
    try {
        shaderQuad = std::make_unique<glowl::GLSLProgram>(glowl::GLSLProgram::ShaderSourceList{
            {glowl::GLSLProgram::ShaderType::Vertex, getStringResource("shaders/quad.vert")},
            {glowl::GLSLProgram::ShaderType::Fragment, getStringResource("shaders/quad.frag")}});
    } catch (glowl::GLSLProgramException& e) {
        std::cerr << e.what() << std::endl;
    }

    // Initialize shader for box rendering
    try {
        shaderBox = std::make_unique<glowl::GLSLProgram>(glowl::GLSLProgram::ShaderSourceList{
            {glowl::GLSLProgram::ShaderType::Vertex, getStringResource("shaders/box.vert")},
            {glowl::GLSLProgram::ShaderType::Fragment, getStringResource("shaders/box.frag")}});
    } catch (glowl::GLSLProgramException& e) {
        std::cerr << e.what() << std::endl;
    }

    // Initialize shader for control point rendering
    try {
        shaderControlPoints = std::make_unique<glowl::GLSLProgram>(glowl::GLSLProgram::ShaderSourceList{
            {glowl::GLSLProgram::ShaderType::Vertex, getStringResource("shaders/control-points.vert")},
            {glowl::GLSLProgram::ShaderType::Fragment, getStringResource("shaders/control-points.frag")}});
    } catch (glowl::GLSLProgramException& e) {
        std::cerr << e.what() << std::endl;
    }

    // Initialize shader for b-spline surface
    // --------------------------------------------------------------------------------
    //  TODO: Implement shader creation for the B-Spline surface shader.
    // --------------------------------------------------------------------------------
}

/**
 * @brief Initialize VAs.
 */
void SurfaceVis::initVAs() {
    // Initialize VA for rendering fbo content
    const std::vector<float> quadVertices{
        // clang-format off
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
        // clang-format on
    };

    const std::vector<GLuint> quadIndices{0, 1, 2, 3};

    glowl::Mesh::VertexDataList<float> vertexDataQuad{{quadVertices, {8, {{2, GL_FLOAT, GL_FALSE, 0}}}}};
    vaQuad = std::make_unique<glowl::Mesh>(vertexDataQuad, quadIndices, GL_UNSIGNED_INT, GL_TRIANGLE_STRIP);

    // Initialize VA for box rendering
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

    glowl::Mesh::VertexDataList<float> vertexDataBox{{boxVertices, {12, {{3, GL_FLOAT, GL_FALSE, 0}}}}};
    vaBox = std::make_unique<glowl::Mesh>(vertexDataBox, boxEdges, GL_UNSIGNED_INT, GL_LINES);

    // Empty VA
    // --------------------------------------------------------------------------------
    //  TODO: Create an empty VA which will be used to draw our b-spline surface.
    //        You must bind the VA once to create it.
    // --------------------------------------------------------------------------------
}

/**
 * @brief Initialize framebuffer object.
 */
void SurfaceVis::initFBO() {
    // --------------------------------------------------------------------------------
    //  TODO: Initialize the fbo (use default depth stencil type):
    //        - 1 color attachment for object colors
    //        - 1 color attachment for picking
    //        Don't forget to check the status of your fbo!
    // --------------------------------------------------------------------------------
}

/**
 * @brief Draw to framebuffer object.
 */
void SurfaceVis::drawToFBO() {
    // --------------------------------------------------------------------------------
    //  TODO: Render to the fbo.
    //        - Draw the box if the variable 'showBox' is true
    //        - Draw the B-Spline surface using the empty vertex array 'emptyVA'
    //        - Draw the control net if 'showControlPoints' is greater than 0.
    //          Use 'pointSize' to set the size of drawn points.
    //          Don't forget to modify the depth test depending on the value of 'showControlPoints'.
    // --------------------------------------------------------------------------------
}

/**
 * @brief Load control points from file.
 * @param filename
 */
void SurfaceVis::loadControlPoints(const std::string& filename) {
    auto path = getResourceDirPath("models") / filename;
    std::cout << "Load model: " << path.string() << std::endl;

    // --------------------------------------------------------------------------------
    //  TODO: Load the control points file from 'path' and initialize all related data.
    // --------------------------------------------------------------------------------
}

/**
 * @brief Save control points to a file.
 * @param filename The name of the file
 */
void SurfaceVis::saveControlPoints(const std::string& filename) {
    auto path = getResourceDirPath("models") / filename;
    std::cout << "Save model: " << path.string() << std::endl;

    // --------------------------------------------------------------------------------
    //  TODO: Save the control points file to 'path'.
    // --------------------------------------------------------------------------------
}

// --------------------------------------------------------------------------------
//  TODO: Implement all self defined methods, e. g. for:
//        - creating and clearing B-Spline related data
//        - updating the position of the picked control point
//        - etc ....
// --------------------------------------------------------------------------------
