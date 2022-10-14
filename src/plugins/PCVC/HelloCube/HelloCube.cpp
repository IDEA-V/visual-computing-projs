#include "HelloCube.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#include "core/Core.h"

using namespace OGL4Core2::Plugins::PCVC::HelloCube;

const static float INIT_CAMERA_DOLLY = 5.0f;
const static float INIT_CAMERA_FOVY = 45.0f;

const static float MIN_CAMERA_PITCH = -89.99f;
const static float MAX_CAMERA_PITCH = 89.99f;

/**
 * @brief HelloCube constructor
 */
HelloCube::HelloCube(const OGL4Core2::Core::Core& c)
    : Core::RenderPlugin(c),
      aspectRatio(1.0f),
      va(0),
      vbo(0),
      ibo(0),
      shaderProgram(0),
      numCubeFaceTriangles(0),
      texture(0),
      projectionMatrix(glm::mat4(1.0f)),
      viewMatrix(glm::mat4(1.0f)),
      modelMatrix(glm::mat4(1.0f)),
      // --------------------------------------------------------------------------------
      //  TODO: Initialize other variables!
      // --------------------------------------------------------------------------------
      backgroundColor(glm::vec3(0.2f, 0.2f, 0.2f)),
      mouseControlMode(0),
      transformCubeMode(0),
      cameraDolly(INIT_CAMERA_DOLLY),
      cameraFoVy(INIT_CAMERA_FOVY),
      cameraPitch(0.0f),
      cameraYaw(0.0f),
      objTranslateX(0.0f),
      objTranslateY(0.0f),
      objTranslateZ(0.0f),
      objRotateX(0.0f),
      objRotateY(0.0f),
      objRotateZ(0.0f),
      objScaleX(1.0f),
      objScaleY(1.0f),
      objScaleZ(1.0f),
      showWireframe(false),
      showTexture(false),
      patternFreq(0) {
    // Init buffer, shader program and texture.
    initBuffers();
    initShaderProgram();
    initTexture();

    // --------------------------------------------------------------------------------
    //  TODO: Set other stuff if necessary!
    // --------------------------------------------------------------------------------

    // Enable depth testing.
    glEnable(GL_DEPTH_TEST);
}

/**
 * @brief HelloCube destructor.
 */
HelloCube::~HelloCube() {
    // --------------------------------------------------------------------------------
    //  TODO: Do not forget to cleanup the plugin!
    // --------------------------------------------------------------------------------

    // Reset OpenGL state.
    glDisable(GL_DEPTH_TEST);
}

/**
 * @brief Render GUI.
 */
void HelloCube::renderGUI() {
    bool cameraChanged = false;
    bool objectChanged = false;
    if (ImGui::CollapsingHeader("Hello Cube", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::ColorEdit3("Background Color", reinterpret_cast<float*>(&backgroundColor), ImGuiColorEditFlags_Float);
        ImGui::Combo("Mouse Control", &mouseControlMode, "Camera\0Object\0");
        if (mouseControlMode == 0) {
            cameraChanged |= ImGui::SliderFloat("Camera Dolly", &cameraDolly, 0.1f, 100.0f);
            cameraChanged |= ImGui::SliderFloat("Camera FoVy", &cameraFoVy, 1.0f, 90.0f);
            cameraChanged |= ImGui::SliderFloat("Camera Pitch", &cameraPitch, MIN_CAMERA_PITCH, MAX_CAMERA_PITCH);
            cameraChanged |= ImGui::SliderFloat("Camera Yaw", &cameraYaw, -360.0f, 360.0f);
        } else {
            ImGui::Combo("TransformCube", &transformCubeMode, "Translate\0Rotate\0Scale\0");
            objectChanged |= ImGui::SliderFloat("Transl. X", &objTranslateX, -10.0f, 10.0f);
            objectChanged |= ImGui::SliderFloat("Transl. Y", &objTranslateY, -10.0f, 10.0f);
            objectChanged |= ImGui::SliderFloat("Transl. Z", &objTranslateZ, -10.0f, 10.0f);
            objectChanged |= ImGui::SliderFloat("Rotate X", &objRotateX, -360.0f, 360.0f);
            objectChanged |= ImGui::SliderFloat("Rotate Y", &objRotateY, -360.0f, 360.0f);
            objectChanged |= ImGui::SliderFloat("Rotate Z", &objRotateZ, -360.0f, 360.0f);
            objectChanged |= ImGui::SliderFloat("Scale X", &objScaleX, 0.1f, 10.0f);
            objectChanged |= ImGui::SliderFloat("Scale Y", &objScaleY, 0.1f, 10.0f);
            objectChanged |= ImGui::SliderFloat("Scale Z", &objScaleZ, 0.1f, 10.0f);
        }
        if (ImGui::Button("Reset cube")) {
            resetCube();
        }
        if (ImGui::Button("Reset camera")) {
            resetCamera();
        }
        if (ImGui::Button("Reset all")) {
            resetCube();
            resetCamera();
        }
        ImGui::Checkbox("Wireframe", &showWireframe);
        ImGui::Checkbox("Texture", &showTexture);
        ImGui::InputInt("Pattern Freq.", &patternFreq);
        patternFreq = std::max(0, std::min(15, patternFreq));
        if (cameraChanged) {
            // --------------------------------------------------------------------------------
            //  TODO: Handle camera parameter changes!
            // --------------------------------------------------------------------------------
        }
        if (objectChanged) {
            // --------------------------------------------------------------------------------
            //  TODO: Handle object parameter changes!
            // --------------------------------------------------------------------------------
        }
    }
}

/**
 * @brief HelloCube render callback.
 */
void HelloCube::render() {
    renderGUI();

    // --------------------------------------------------------------------------------
    //  TODO: Implement rendering!
    // --------------------------------------------------------------------------------
}

/**
 * @brief HelloCube resize callback.
 * @param width
 * @param height
 */
void HelloCube::resize(int width, int height) {
    // --------------------------------------------------------------------------------
    //  TODO: Handle resize event!
    // --------------------------------------------------------------------------------
}

/**
 * @brief HellCube mouse move callback.
 * @param xpos
 * @param ypos
 */
void HelloCube::mouseMove(double xpos, double ypos) {
    // --------------------------------------------------------------------------------
    //  TODO: Handle camera and object control!
    // --------------------------------------------------------------------------------
}

/**
 * @brief Initialize buffers.
 */
void HelloCube::initBuffers() {
    // --------------------------------------------------------------------------------
    //  TODO: Init vertex buffer and index buffer!
    // --------------------------------------------------------------------------------
    const int numCubeVertices = 8;

    const std::vector<float> cubeVertices{
        -0.5f, -0.5f, -0.5f, 1.0f, // left  bottom back
        0.5f, -0.5f, -0.5f, 1.0f, // right bottom back
        -0.5f,  0.5f, -0.5f, 1.0f, // left  top    back
        0.5f,  0.5f, -0.5f, 1.0f, // right top    back
        -0.5f, -0.5f,  0.5f, 1.0f, // left  bottom front
        0.5f, -0.5f,  0.5f, 1.0f, // right bottom front
        -0.5f,  0.5f,  0.5f, 1.0f, // left  top    front
        0.5f,  0.5f,  0.5f, 1.0f, // right top    front
    };

    numCubeFaceTriangles = 12;

    const std::vector<GLuint> cubeFaces{
        0, 2, 1,  1, 2, 3, // back
        5, 7, 4,  4, 7, 6, // front
        1, 5, 0,  0, 5, 4, // bottom
        7, 3, 6,  6, 3, 2, // top
        4, 6, 0,  0, 6, 2, // left
        1, 3, 5,  5, 3, 7  // right
    };

}


/**
 * @brief HelloCube init shaders
 */
void HelloCube::initShaderProgram() {
    // --------------------------------------------------------------------------------
    //  TODO: Init shader program!
    // --------------------------------------------------------------------------------
}

/**
 * @brief Initialize textures.
 */
void HelloCube::initTexture() {
    // --------------------------------------------------------------------------------
    //  TODO: Init texture!
    // --------------------------------------------------------------------------------
    int width, height;
    auto image = getPngResource("texture.png", width, height);

}

/**
 * @brief Reset cube.
 */
void HelloCube::resetCube() {
    // --------------------------------------------------------------------------------
    //  TODO: Reset cube!
    // --------------------------------------------------------------------------------
}

/**
 * @brief Reset camera.
 */
void HelloCube::resetCamera() {
    // --------------------------------------------------------------------------------
    //  TODO: Reset camera!
    // --------------------------------------------------------------------------------
}

/**
 * @brief Set projection matrix.
 */
void HelloCube::setProjectionMatrix() {
    // --------------------------------------------------------------------------------
    //  TODO: Set projection matrix!
    // --------------------------------------------------------------------------------
}

/**
 * @brief Set view matrix.
 */
void HelloCube::setViewMatrix() {
    // --------------------------------------------------------------------------------
    //  TODO: Set view matrix!
    // --------------------------------------------------------------------------------
}

/**
 * @brief Set model matrix.
 */
void HelloCube::setModelMatrix() {
    // --------------------------------------------------------------------------------
    //  TODO: Set model matrix!
    // --------------------------------------------------------------------------------
}
