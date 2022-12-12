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
#include <glm/gtx/string_cast.hpp>

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
      zNear(0.01f),
      zFar(10.0f),
      projMx(glm::mat4(1.0f)),
      vaEmpty(0),
      maxTessGenLevel(64),
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
      pickedId(0),
      moveMode(0),
      tessLevelInner(16),
      tessLevelOuter(16),
      numControlPoints_n(4),
      numControlPoints_m(4),
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
    glGenBuffers(1, &UBuffer);
    glGenBuffers(1, &VBuffer);
    glGenBuffers(1, &PBUffer);

    // --------------------------------------------------------------------------------
    //  TODO: Initialize a flat b-spline surface.
    // --------------------------------------------------------------------------------
    initControlPoints();
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
    bool nChanged = ImGui::InputInt("numControlPoints_n", &numControlPoints_n);
    numControlPoints_n = std::clamp(numControlPoints_n, 2, 8);
    bool mChanged = ImGui::InputInt("numControlPoints_m", &numControlPoints_m);
    numControlPoints_m = std::clamp(numControlPoints_m, 2, 8);

    if (nChanged | mChanged) {
        initControlPoints();
    }

    bool pickedChanged = ImGui::InputInt("pickedID", &pickedId);
    pickedId = std::clamp(pickedId, 0, numControlPoints_n*numControlPoints_m);

    if (pickedChanged) {
        if (pickedId > 0) {
            for (int i = 0; i < 3; i++) pickedPosition[i] = controlPointsVertices[(pickedId - 1) * 3 + i];
        }
        else {
            for (int i = 0; i < 3; i++) pickedPosition[i] = 0.0f;
        }
    }

    bool pickedPosChanged = ImGui::DragFloat3("pickedPosition", pickedPosition, 0.01f, -5.0f, 5.0f);
    if (pickedPosChanged) {
        if (pickedId > 0) {
            for (int i = 0; i < 3; i++) controlPointsVertices[(pickedId - 1) * 3 + i] = pickedPosition[i];
        }
        // else {
        //     for (int i = 0; i < 3; i++) pickedPosition[i] = 0.0f;
        // }
    }

    ImGui::InputInt("tessLevelInner", &tessLevelInner);
    tessLevelInner = std::clamp(tessLevelInner, 1, maxTessGenLevel);
    ImGui::InputInt("tessLevelOuter", &tessLevelOuter);
    tessLevelOuter = std::clamp(tessLevelOuter, 1, maxTessGenLevel);

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
    projMx = glm::perspective(glm::radians(fovY), (float) wWidth / (float)wHeight, zNear, zFar);
    drawToFBO();


    glViewport(0, 0, wWidth, wHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 orthoMx = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f);

    shaderQuad->use();
    shaderQuad->setUniform("projMx", orthoMx);

    // --------------------------------------------------------------------------------
    //  TODO: Draw the screen-filling quad using the previous fbo as input texture.
    // --------------------------------------------------------------------------------
    glActiveTexture(GL_TEXTURE0);
    fbo->bindColorbuffer(0);
    shaderQuad->setUniform("tex", 0);
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
        initFBO();
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
    if (key == Core::Key::R) {
        initShaders();
    }
    else if (key == Core::Key::X) {
        pickedId = 0;
        for (int i = 0; i < 3; i++) pickedPosition[i] = 0;
    }
    else if (key == Core::Key::Right) {
        if (pickedId < numControlPoints_n*numControlPoints_m) {
            pickedId++;
            for (int i = 0; i < 3; i++) pickedPosition[i] = controlPointsVertices[(pickedId - 1) * 3 + i];
        }
    }
    else if (key == Core::Key::Left) {
        if (pickedId > 1) {
            pickedId--;
            for (int i = 0; i < 3; i++) pickedPosition[i] = controlPointsVertices[(pickedId - 1) * 3 + i];
        }
    }
    else if (key == Core::Key::L) {
        loadControlPoints(dataFilename);
    }
    else if (key == Core::Key::S) {
        saveControlPoints(dataFilename);
    }
    else if (key == Core::Key::Backspace) {
        initControlPoints();
        // initKnotVector();
    }
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
    if ((action == Core::MouseButtonAction::Press) && mods.onlyControl() && (button == Core::MouseButton::Left)) {
        fbo->bindToRead(1);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        unsigned char data[3];
        glReadPixels(lastMouseX, wHeight - lastMouseY, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, data);
        pickedId = colorToId(data);
        if (pickedId > 0) {
            for (int i = 0; i < 3; i++) pickedPosition[i] = controlPointsVertices[(pickedId - 1) * 3 + i];
        }
        else {
            for (int i = 0; i < 3; i++) pickedPosition[i] = 0.0f;
        }
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    }else if ((pickedId > 0) && (action == Core::MouseButtonAction::Press) && mods.onlyControl() && (button == Core::MouseButton::Middle)) moveMode = 1;
    else if ((pickedId > 0) && (action == Core::MouseButtonAction::Press) && mods.onlyControl() && (button == Core::MouseButton::Right)) moveMode = 2;
    else moveMode = 0;
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
    if (moveMode > 0) {
        // World coord. to screen coord.
        glm::vec4 worldPos = glm::vec4(pickedPosition[0], pickedPosition[1], pickedPosition[2], 1.0f);
        glm::vec4 clipPos = projMx * camera->viewMx() * worldPos;
        glm::vec4 ndcPos = clipPos / clipPos.w;
        glm::vec3 screenPos = glm::vec3((ndcPos.x + 1.0) * wWidth / 2.0f, -(ndcPos.y - 1.0f) * wHeight / 2.0f, ndcPos.z * (zFar - zNear) / 2.0f + + (zFar + zNear) / 2.0f);

        // Move the selected control point in the xy-plane
        if (moveMode == 1) {
            screenPos += glm::vec3(xpos - lastMouseX, ypos - lastMouseY, 0.0f);
        }
        // Move the selected control point in the z-plane
        else if (moveMode == 2) {
            screenPos += glm::vec3(0.0f, 0.0f, (ypos - lastMouseY) * -0.0001f);
        }

        // Screen coord. to world coord.
        ndcPos = glm::vec4(2.0f * screenPos.x / wWidth - 1.0f, 1.0f - 2.0f * screenPos.y / wHeight, (2.0f * screenPos.z - (zFar + zNear)) / (zFar - zNear), 1.0f);
        clipPos = ndcPos;
        worldPos = inverse(projMx * camera->viewMx()) * clipPos;
        worldPos = worldPos / worldPos.w;

        // Update vertex array
        for (int i = 0; i < 3; i++) {
            pickedPosition[i] = worldPos[i];
            controlPointsVertices[3 * (pickedId - 1) + i] = pickedPosition[i];
        }
    }
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
    try {
        shaderBSplineSurface = std::make_unique<glowl::GLSLProgram>(glowl::GLSLProgram::ShaderSourceList{
            {glowl::GLSLProgram::ShaderType::Vertex, getStringResource("shaders/surface.vert")},
            {glowl::GLSLProgram::ShaderType::TessControl, getStringResource("shaders/surface.tesc")},
            {glowl::GLSLProgram::ShaderType::TessEvaluation, getStringResource("shaders/surface.tese")},
            {glowl::GLSLProgram::ShaderType::Fragment, getStringResource("shaders/surface.frag")} });
    }
    catch (glowl::GLSLProgramException& e) { std::cerr << e.what() << std::endl; }
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
    glGenVertexArrays(1, &vaEmpty);
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
    fbo = std::make_unique<glowl::FramebufferObject>(wWidth, wHeight, glowl::FramebufferObject::DEPTH24);
    fbo->bind();
    fbo->createColorAttachment(GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE); // Object color
    fbo->createColorAttachment(GL_RGB32F, GL_RGB, GL_UNSIGNED_INT); // Picking
    GLenum status = fbo->checkStatus();
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

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void SurfaceVis::initControlPoints() {
    // Clear the va, idColor, index
    controlPointsVertices.clear();
    controlPointsColor.clear();
    controlPointsIndices.clear();

    // Create control points
    float step_n = 1.0f / (numControlPoints_n - 1);
    float step_m = 1.0f / (numControlPoints_m - 1);
    for (int i = 0; i < numControlPoints_n; i++) {
        glm::vec3 startPoint = glm::vec3(-0.5f, 0.5f, 0.0f) - glm::vec3(0.0f, step_n * i, 0.0f);
        for (int j = 0; j < numControlPoints_m; j++) {
            // Vertex position
            controlPointsVertices.push_back(startPoint.x + step_m * j);
            controlPointsVertices.push_back(startPoint.y);
            controlPointsVertices.push_back(startPoint.z);

            // index
            int index = numControlPoints_m * i + j;
            if (j > 0) {
                controlPointsIndices.push_back(index - 1);
                controlPointsIndices.push_back(index);
            }
            if (i > 0) {
                controlPointsIndices.push_back(index - numControlPoints_m);
                controlPointsIndices.push_back(index);
            }

            // ID color
            glm::vec3 idColor = idToColor(index+1);
            controlPointsColor.push_back(idColor.x);
            controlPointsColor.push_back(idColor.y);
            controlPointsColor.push_back(idColor.z);
        }
    }
    initKnotVectors();
}

// float N_function(std::vector<float> U, int i, int p) {
//     if (p==0) {
//         return 
//     }
// }

float N (std::vector<float> U, int i, int p, float u) {
    if (p==0) {
        if (u >= U[i] && u < U[i+1]) return 1.0f;
        else return 0.0f;
    }else{
        float quotient1;
        float quotient2; 
        if (U[i+3] - U[i] < 0.001) quotient1 = 0;
        else quotient1 = (u-U[i])/(U[i+3] - U[i]);
        if (U[i+3+1] - U[i+1] < 0.001) quotient2 = 0;
        else quotient2 = (U[i+3+1] - u)/(U[i+3+1] - U[i+1]);

        return quotient1*N(U, i, p-1, u) + quotient2*N(U, i+1, p-1, u);
    }
}

void SurfaceVis::initKnotVectors() {
    std::vector<float> U;
    std::vector<float> V;

    for (int i = 0; i < 4; i++)
    {
        U.push_back(0.0f);
        V.push_back(0.0f);
    }
    float stepN = 1/(numControlPoints_n -2);
    float stepM = 1/(numControlPoints_m -2);
    float knotV = 0.0f;
    for (int i = 0; i < numControlPoints_n-3; i++)
    {
        knotV += stepN;
        U.push_back(knotV);
    }
    knotV = 0.0f;
    for (int i = 0; i < numControlPoints_m-3; i++)
    {
        knotV += stepM;
        V.push_back(knotV);
    }
    for (int i = 0; i < 4; i++)
    {
        U.push_back(1.0f);
        V.push_back(1.0f);
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, UBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * U.size(), U.data(), GL_STATIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, VBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * V.size(), V.data(), GL_STATIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, PBUffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * controlPointsVertices.size(), controlPointsVertices.data(), GL_STATIC_COPY);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, PBUffer);
    float Us[U.size()];
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(float)*U.size(), Us);
    for (size_t i = 0; i < 3*numControlPoints_n * numControlPoints_m; i++)
    {
        std::cout << Us[i] << " ";
    }
    std::cout << std::endl;

    // for (int i = 0; i <= numControlPoints_n; i++) {
    //     for (int j = 0; j <= numControlPoints_m; j++) {
    //         int index = numControlPoints_m * i + j;
    //         std::cout << Us[3*index] << " " << Us[3*index+1] << " " << Us[3*index+2] << std::endl;
    //         // glm::vec3 pij = glm::vec3(P[3*index], P[3*index+1], P[3*index+1]);
    //         // S += N3(0, i, gl_TessCoord.x)*N3(1, j, gl_TessCoord.y)*pij;
    //     }
    // }

    glm::vec3 S;
    for (int i = 0; i <= numControlPoints_n; i++) {
        for (int j = 0; j <= numControlPoints_m; j++) {
            int index = numControlPoints_m * i + j;
            glm::vec3 pij = glm::vec3(controlPointsVertices[3*index], controlPointsVertices[3*index+1], controlPointsVertices[3*index+2]);
            S += N(U, i, 3, 0.5)*N(V, j, 3, 0.5)*pij;
        }
    }

    std::cout << glm::to_string(S) <<std::endl;

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
    fbo->bindToDraw();

    // glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    char bg[3] = {55,55,55};
    fbo->getColorAttachment(0)->clearTexImage(bg);

    if (showBox) {
        shaderBox->use();
        shaderBox->setUniform("projMx", projMx);
        shaderBox->setUniform("viewMx", camera->viewMx());
        vaBox->draw();
        glUseProgram(0);
    }

    glowl::Mesh::VertexDataList<float> vDataControlPoint{{controlPointsVertices, {12, {{3, GL_FLOAT, GL_FALSE, 0}}}}, {controlPointsColor, {12, {{3, GL_FLOAT, GL_FALSE, 0}}}}};

    vaControlPoints = std::make_unique<glowl::Mesh>(vDataControlPoint, controlPointsIndices, GL_UNSIGNED_INT, GL_POINTS);
    vaControlPoints_LINES = std::make_unique<glowl::Mesh>(vDataControlPoint, controlPointsIndices, GL_UNSIGNED_INT, GL_LINES);

    if (showControlPoints > 0) {
        if (showControlPoints == 2) glDepthMask(GL_FALSE);
        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
        glEnable(GL_LINE_SMOOTH);
        shaderControlPoints->use();
        shaderControlPoints->setUniform("projMx", projMx);
        shaderControlPoints->setUniform("viewMx", camera->viewMx());
        shaderControlPoints->setUniform("pickedIdCol", idToColor(pickedId));
        shaderControlPoints->setUniform("pointSize", pointSize);
        vaControlPoints->draw();
        vaControlPoints_LINES->draw();
        glUseProgram(0);
        glDepthMask(GL_TRUE);
    }

    // Check whether the Wireframe checkbox is checked
    if (useWireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    // Draw the B-Spline surface
    shaderBSplineSurface->use();
    glBindVertexArray(vaEmpty);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, UBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, UBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, VBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, VBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, PBUffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, PBUffer);

    shaderBSplineSurface->setUniform("tessLevelInner", tessLevelInner);
    shaderBSplineSurface->setUniform("tessLevelOuter", tessLevelOuter);
    shaderBSplineSurface->setUniform("projMx", projMx);
    shaderBSplineSurface->setUniform("viewMx", camera->viewMx());
    shaderBSplineSurface->setUniform("showNormals", showNormals);
    shaderBSplineSurface->setUniform("freq", freq);
    shaderBSplineSurface->setUniform("n", numControlPoints_n);
    shaderBSplineSurface->setUniform("m", numControlPoints_m);

    glPatchParameteri(GL_PATCH_VERTICES, 4); // Number of the vertices per patch
    glDrawArraysInstanced(GL_PATCHES, 0, 4, 1); // (primitives type, started index, #vertex * #patch, #draw)
    glBindVertexArray(0);
    glUseProgram(0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
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
    std::ifstream file(path);
    if (!file.good()) {
        std::cout << "file doesn't exist!" << std::endl;
        return;
    }
    file >> numControlPoints_n >> degree_p >> numControlPoints_m >> degree_q;

    // Load vertex
    float coord;
    controlPointsVertices.clear();
    while (file >> coord) {
        controlPointsVertices.push_back(coord);
    }
    file.close();

    // Init. index and idColor
    std::vector<GLuint> controlPointsIndices;
    std::vector<float> controlPointsColor;
    for (int i = 0; i < numControlPoints_n; i++) {
        for (int j = 0; j < numControlPoints_m; j++) {
            // Index
            int index = numControlPoints_m * i + j;
            if (j > 0) {
                controlPointsIndices.push_back(index - 1);
                controlPointsIndices.push_back(index);
            }
            if (i > 0) {
                controlPointsIndices.push_back(index - numControlPoints_m);
                controlPointsIndices.push_back(index);
            }

            // ID color
            glm::vec3 idColor = idToColor(index + 1);
            controlPointsColor.push_back(idColor.x);
            controlPointsColor.push_back(idColor.y);
            controlPointsColor.push_back(idColor.z);
        }
    }

    glowl::Mesh::VertexDataList<float> vDataControlPoints{{controlPointsVertices, {12, {{3, GL_FLOAT, GL_FALSE, 0}}}}, {controlPointsColor, {12, {{3, GL_FLOAT, GL_FALSE, 0}}}}};
    
    vaControlPoints = std::make_unique<glowl::Mesh>(vDataControlPoints, controlPointsIndices, GL_UNSIGNED_INT, GL_POINTS);
    vaControlPoints_LINES = std::make_unique<glowl::Mesh>(vDataControlPoints, controlPointsIndices, GL_UNSIGNED_INT, GL_LINES);
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
    std::ofstream file(path);
    file << numControlPoints_n << " " << degree_p << std::endl;
    file << numControlPoints_m << " " << degree_q << std::endl;
    file << std::showpoint;
    for (float i = 0; i < numControlPoints_n*numControlPoints_m; i++) {
        file << std::setw(10) << controlPointsVertices[3 * i] << " " << controlPointsVertices[3 * i + 1] << " "
            << controlPointsVertices[3 * i + 2] << std::endl;
    }
    file.close();
}

// --------------------------------------------------------------------------------
//  TODO: Implement all self defined methods, e. g. for:
//        - creating and clearing B-Spline related data
//        - updating the position of the picked control point
//        - etc ....
// --------------------------------------------------------------------------------
