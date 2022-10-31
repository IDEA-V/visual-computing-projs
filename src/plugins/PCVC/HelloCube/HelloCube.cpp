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
using namespace glm;

const static float INIT_CAMERA_DOLLY = 5.0f;
const static float INIT_CAMERA_FOVY = 45.0f;
const static float INIT_CAMERA_PITCH = -30.0f;
const static float INIT_CAMERA_YAW = -45.0f;

const static float MIN_CAMERA_PITCH = -89.99f;
const static float MAX_CAMERA_PITCH = 89.99f;

const static float zFar = 100.0f;
const static float zNear = 0.1f;

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
      vertexShader(0),
      fragmentShader(0),
      lastMouseX(0),
      lastMouseY(0),
      windowWidth(0),
      windowHeight(0),
      // --------------------------------------------------------------------------------
      backgroundColor(glm::vec3(0.2f, 0.2f, 0.2f)),
      mouseControlMode(0),
      transformCubeMode(0),
      cameraDolly(INIT_CAMERA_DOLLY),
      cameraFoVy(INIT_CAMERA_FOVY),
      cameraPitch(INIT_CAMERA_PITCH),
      cameraYaw(INIT_CAMERA_YAW),
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
    std::cout<< "Quitting!" << std::endl;
    //delete shaders and the program. Order matters!
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteProgram(shaderProgram);
    //delete buffer objects
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
    //delete vertax array objects
    glDeleteVertexArrays(1 , &va);
    glDeleteTextures(1, &texture);
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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], 1.0f);

    // show wireframe if checked
    if (showWireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    GLint patternFreqUniformLocation = glGetUniformLocation(shaderProgram, "patternFreq");
    glUniform1i(patternFreqUniformLocation, patternFreq);

    //pass showTextureInt to fragment shader
    GLint textureUniformLocation = glGetUniformLocation(shaderProgram, "showTextureInt");
    glUniform1i(textureUniformLocation, (GLint)showTexture);

    //pass three matrix to vertex shader
    //matrix computation is faster on GPU so the matrix are passed without computation
    setProjectionMatrix();
    setViewMatrix();
    setModelMatrix();

    // Draw the object
    glBindVertexArray(va);
    glDrawElements(GL_TRIANGLES, 3 * numCubeFaceTriangles, GL_UNSIGNED_INT, NULL);
}

/**
 * @brief HelloCube resize callback.
 * @param width
 * @param height
 */
void HelloCube::resize(int width, int height) {
    //reset view port and save window size to resize the cube later
    glViewport(0, 0, width, height);
    windowWidth = width;
    windowHeight = height;
}

/**
 * @brief HellCube mouse move callback.
 * @param xpos
 * @param ypos
 */
void HelloCube::mouseMove(double xpos, double ypos) {
    if (this->core_.isMouseButtonPressed(Core::MouseButton::Left) | this->core_.isMouseButtonPressed(Core::MouseButton::Right) | this->core_.isMouseButtonPressed(Core::MouseButton::Middle)) {
        int moveX = 0;
        int moveY = 0;

        if (xpos - lastMouseX == -1) moveX = -1;
        if (xpos - lastMouseX == 1) moveX = 1;
        if (ypos - lastMouseY == -1) moveY = -1;
        if (ypos - lastMouseY == 1) moveY = 1;

        // std::cout << moveX << ',' << moveY << std::endl;

        if (mouseControlMode == 0) {//Camera control
            if (this->core_.isMouseButtonPressed(Core::MouseButton::Left)) {
                cameraYaw = std::min(std::max(cameraYaw - moveX * 0.5f, -360.0f), 360.0f);
                cameraPitch = std::min(std::max(cameraPitch - moveY * 0.5f, MIN_CAMERA_PITCH), MAX_CAMERA_PITCH);
            }else if (this->core_.isMouseButtonPressed(Core::MouseButton::Right)) {
                cameraDolly = std::min(std::max(cameraDolly - moveY * 0.1f, 0.1f), 100.0f);
            }else if (this->core_.isMouseButtonPressed(Core::MouseButton::Middle)) {
                cameraFoVy = std::min(std::max(cameraFoVy - moveY * 0.5f, 0.1f), 90.0f);
            }
        }else{ //Cube control
            if (transformCubeMode == 0) { // Translate
                if (this->core_.isMouseButtonPressed(Core::MouseButton::Left) | this->core_.isMouseButtonPressed(Core::MouseButton::Right)) {
                    objTranslateX = std::min(std::max(objTranslateX + moveX * 0.01f, -10.f), 10.0f);
                    if (this->core_.isMouseButtonPressed(Core::MouseButton::Left)){
                        objTranslateY = std::min(std::max(objTranslateY + moveY * 0.01f, -10.f), 10.0f);
                    }else{
                        objTranslateZ = std::min(std::max(objTranslateZ + moveY * 0.01f, -10.f), 10.0f);
                    }
                }else{
                    objTranslateY = std::min(std::max(objTranslateY + moveX * 0.01f, -10.f), 10.0f);
                    objTranslateZ = std::min(std::max(objTranslateZ + moveY * 0.01f, -10.f), 10.0f);
                }
            } else if (transformCubeMode == 1) { // Rotate
                if (this->core_.isMouseButtonPressed(Core::MouseButton::Left) | this->core_.isMouseButtonPressed(Core::MouseButton::Right)) {
                    objRotateY = std::min(std::max(objRotateY + moveX * 0.5f, -360.f), 360.f);
                    if (this->core_.isMouseButtonPressed(Core::MouseButton::Left)){
                        objRotateX = std::min(std::max(objRotateX + moveY * 0.5f, -360.f), 360.f);
                    }else{
                        objRotateZ = std::min(std::max(objRotateZ + moveY * 0.5f, -360.f), 360.f);
                    }
                }else{
                    objRotateX = std::min(std::max(objRotateX + moveX * 0.5f, -360.f), 360.f);
                    objRotateZ = std::min(std::max(objRotateZ + moveY * 0.5f, -360.f), 360.f);
                }
            } else if (transformCubeMode == 2) { // Scale
                if (this->core_.isMouseButtonPressed(Core::MouseButton::Left) | this->core_.isMouseButtonPressed(Core::MouseButton::Right)) {
                    objScaleX = std::min(std::max(objScaleX + moveX * 0.01f, 0.1f), 10.0f);
                    if (this->core_.isMouseButtonPressed(Core::MouseButton::Left)){
                        objScaleY = std::min(std::max(objScaleY + moveY * 0.01f, 0.1f), 10.0f);
                    }else{
                        objScaleZ = std::min(std::max(objScaleZ + moveY * 0.01f, 0.1f), 10.0f);
                    }
                }else{
                    objScaleY = std::min(std::max(objScaleY + moveX * 0.01f, 0.1f), 10.0f);
                    objScaleZ = std::min(std::max(objScaleZ + moveY * 0.01f, 0.1f), 10.0f);
                }
            }
        }

        lastMouseX = xpos;
        lastMouseY = ypos;

    }
}

/**
 * @brief Initialize buffers.
 */
void HelloCube::initBuffers() {

    const std::vector<float> cubeVertices{
        //bottom four points          
        -0.5f, -0.5f, -0.5f, 0.25f, 0.75f,  //0 bottom
        -0.5f, -0.5f,  0.5f, 0.25f, 0.5f,   //1 bottom, front, left 
         0.5f, -0.5f,  0.5f, 0.5f,  0.5f,   //2 bottom, fromt, right
         0.5f, -0.5f, -0.5f, 0.5f,  0.75f,  //3 bottom
        //top four points
        -0.5f,  0.5f, -0.5f, 0.25f, 0.0f,   //4 top
        -0.5f,  0.5f,  0.5f, 0.25f, 0.25f,  //5 top, front, left
         0.5f,  0.5f,  0.5f, 0.5f,  0.25f,  //6 top, front, right
         0.5f,  0.5f, -0.5f, 0.5f,  0.0f,   //7 top

        -0.5f,  0.5f, -0.5f, 0.0f,  0.25f,  //8 left 4'
        -0.5f, -0.5f, -0.5f, 0.0f,  0.5f,   //9 left 0'

         0.5f,  0.5f, -0.5f, 0.75f, 0.25f,  //10 right back 7'
         0.5f, -0.5f, -0.5f, 0.75f, 0.5f,   //11 right back 3'
        -0.5f,  0.5f, -0.5f, 1.0f,  0.25f,  //12 back 4'
        -0.5f, -0.5f, -0.5f, 1.0f,  0.5f,   //13 back 0'
    };

    numCubeFaceTriangles = 12;

    const std::vector<GLuint> cubeFaces{
        0, 3, 1, 1, 3, 2,  //bottom
        1, 2, 5, 2, 5, 6,  //front
        4, 5, 6, 4, 6, 7,  //top
        8, 9, 1, 8, 1, 5,  //left
        6, 2, 11,6, 11,10, //right
        10,11,13,10,13,12  //back
    };

    // vertex array object should be bind first
    glGenVertexArrays(1, &va);
    glBindVertexArray(va);

    // Set up vertext buffer
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, cubeVertices.size() * sizeof(float), &cubeVertices[0], GL_STATIC_DRAW);

    // Set up index buffer
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubeFaces.size() * sizeof(int), &cubeFaces[0], GL_STATIC_DRAW);

    // define position attribute and enable it
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // define textureCoord attribute and enable it
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

}


/**
 * @brief HelloCube init shaders
 */
void HelloCube::initShaderProgram() {
    //get shader code and turn to char*
    std::string vertexShaderString = getStringResource(getResourcePath("../resources/cube.vert").string());
    std::string fragmentShaderString = getStringResource(getResourcePath("../resources/cube.frag").string());
    const char* vertexShaderCode = vertexShaderString.c_str();
    const char* fragmentShaderCode = fragmentShaderString.c_str();

    //create vertex shader and compile it
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderCode, NULL);
    glCompileShader(vertexShader);

    //vertex shader error handler
    int success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        int length;
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &length);
        char* message = new char[length];
        glGetShaderInfoLog(vertexShader, length, &length, message);
        std::string s = message;
        s = "Vertex shader compilation failed:\n" + s;
        delete message;
        glDisable(GL_DEPTH_TEST);
        std::cout<< "Quitting!" << std::endl;
        //delete shaders and the program. Order matters!
        glDeleteShader(vertexShader);
        //delete buffer objects
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ibo);
        //delete vertax array objects
        glDeleteVertexArrays(1 , &va);
        throw std::runtime_error(s);
    }

    //create fragment shader and compile it
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderCode, NULL);
    glCompileShader(fragmentShader);

    //fragment shader error handler
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        int length;
        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &length);
        char* message = new char[length];
        glGetShaderInfoLog(fragmentShader, length, &length, message);
        std::string s = message;
        s = "Fragment shader compilation failed:\n" + s;
        delete message;
        glDisable(GL_DEPTH_TEST);
        std::cout<< "Quitting!" << std::endl;
        //delete shaders and the program. Order matters!
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        //delete buffer objects
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ibo);
        //delete vertax array objects
        glDeleteVertexArrays(1 , &va);
        throw std::runtime_error(s);
    }

    //attach shaders and link program
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // shader program error handler
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        int length;
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &length);
        char* message = new char[length];
        glGetProgramInfoLog(shaderProgram, length, &length, message);
        std::string s = message;
        s = "Shader program link failed:\n" + s;
        delete message;
        glDisable(GL_DEPTH_TEST);
        std::cout<< "Quitting!" << std::endl;
        //delete shaders and the program. Order matters!
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(shaderProgram);
        //delete buffer objects
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ibo);
        //delete vertax array objects
        glDeleteVertexArrays(1 , &va);
        throw std::runtime_error(s);
    }

    glUseProgram(shaderProgram);
}

/**
 * @brief Initialize textures.
 */
void HelloCube::initTexture() {
    int width, height;
    auto image = getPngResource(getResourcePath("../resources/texture.png").string(), width, height);
    
    // debug code to show image fetched
    // std::cout << width <<',' << height << std::endl;
    // std::cout << image.size() << std::endl;
    // auto im = Mat(1024,1024, CV_8UC4, image.data());
    // std::cout << im.size << std::endl;

    // imshow("color image",im);
    // waitKey(0);
    
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

}

/**
 * @brief Reset cube.
 */
void HelloCube::resetCube() {
    // --------------------------------------------------------------------------------
    //  TODO: Reset cube!
    // --------------------------------------------------------------------------------
    objTranslateX = 0.0f;
    objTranslateY = 0.0f;
    objTranslateZ = 0.0f;
    objRotateX = 0.0f;
    objRotateY = 0.0f;
    objRotateZ = 0.0f;
    objScaleX = 1.0f;
    objScaleY = 1.0f;
    objScaleZ = 1.0f;
}

/**
 * @brief Reset camera.
 */
void HelloCube::resetCamera() {
    // --------------------------------------------------------------------------------
    //  TODO: Reset camera!
    // --------------------------------------------------------------------------------
    cameraDolly = INIT_CAMERA_DOLLY;
    cameraFoVy = INIT_CAMERA_FOVY;
    cameraPitch = INIT_CAMERA_PITCH;
    cameraYaw = INIT_CAMERA_YAW;
}

/**
 * @brief Set projection matrix.
 */
void HelloCube::setProjectionMatrix() {
    // --------------------------------------------------------------------------------
    //  TODO: Set projection matrix!
    // --------------------------------------------------------------------------------
    float tanf = tan(radians(cameraFoVy / 2));
    float aspect =  (float)windowWidth / (float)windowHeight;
    projectionMatrix = {
        1.0f / (aspect * tanf), 0.0f,       0.0f,                               0.0f,
        0.0f,                   1.0 / tanf, 0.0f,                               0.0f,
        0.0f,                   0.0f,       -(zFar + zNear) / (zFar - zNear),   -1.0,
        0.0f,                   0.0f,       -2 * zFar * zNear / (zFar - zNear), 0.0f
    };

    // Pass projection matrix to cube.vert
    GLint location = glGetUniformLocation(shaderProgram, "projectionMatrix");
    glUniformMatrix4fv(location, 1 , GL_FALSE, &projectionMatrix[0][0]);
}

/**
 * @brief Set view matrix.
 */
void HelloCube::setViewMatrix() {
    // --------------------------------------------------------------------------------
    //  TODO: Set view matrix!
    // --------------------------------------------------------------------------------
    mat4 rotationX = {
        1.0f, 0.0f,                      0.0f,                       0.0f,
        0.0f, cos(radians(cameraPitch)), -sin(radians(cameraPitch)), 0.0f,
        0.0f, sin(radians(cameraPitch)), cos(radians(cameraPitch)),  0.0f,
        0.0f, 0.0f,                      0.0f,                       1.0f
    };

    // Rotation matrix for y-asis
    mat4 rotationY = {
        cos(radians(cameraYaw)),  0.0f, sin(radians(cameraYaw)), 0.0f,
        0.0f,                     1.0f, 0.0f,                    0.0f,
        -sin(radians(cameraYaw)), 0.0f, cos(radians(cameraYaw)), 0.0f,
        0.0f,                     0.0f, 0.0f,                    1.0f
    };

    // Translation matrix
    mat4 translation = {
        1.0f, 0.0f, 0.0f,  0.0f,
        0.0f, 1.0f, 0.0f,  0.0f,
        0.0f, 0.0f, 1.0f,  0.0f,
        0.0f, 0.0f, -cameraDolly, 1.0f
    };

    viewMatrix = translation * rotationX * rotationY * mat4(1.0);

    // Pass view matrix to cube.vert
    GLint location = glGetUniformLocation(shaderProgram, "viewMatrix");
    glUniformMatrix4fv(location, 1, GL_FALSE, &viewMatrix[0][0]);
}

/**
 * @brief Set model matrix.
 */
void HelloCube::setModelMatrix() {
    // --------------------------------------------------------------------------------
    //  TODO: Set model matrix!
    // --------------------------------------------------------------------------------
    mat4 translation = {
        1.0f,          0.0f,          0.0f,          0.0f,
        0.0f,          1.0f,          0.0f,          0.0f,
        0.0f,          0.0f,          1.0f,          0.0f,
        objTranslateX, objTranslateY, objTranslateZ, 1.0f
    };

    // Rotation matrix for x-asis
    mat4 rotationX = {
        1.0f, 0.0f,                     0.0f,                      0.0f,
        0.0f, cos(radians(objRotateX)), -sin(radians(objRotateX)), 0.0f,
        0.0f, sin(radians(objRotateX)), cos(radians(objRotateX)),  0.0f,
        0.0f, 0.0f,                     0.0f,                      1.0f
    };

    // Rotation matrix for y-asis
    mat4 rotationY = {
        cos(radians(objRotateY)),  0.0f, sin(radians(objRotateY)), 0.0f,
        0.0f,                      1.0f, 0.0f,                     0.0f,
        -sin(radians(objRotateY)), 0.0f, cos(radians(objRotateY)), 0.0f,
        0.0f,                      0.0f, 0.0f,                     1.0f
    };

    // Rotation matrix for z-asis
    mat4 rotationZ = {
        cos(radians(objRotateZ)), -sin(radians(objRotateZ)), 0.0f, 0.0f,
        sin(radians(objRotateZ)), cos(radians(objRotateZ)),  0.0f, 0.0f,
        0.0f,                     0.0f,                      1.0f, 0.0f,
        0.0f,                     0.0f,                      0.0f, 1.0f
    };
    mat4 rotateMatrix = rotationX * rotationY * rotationZ;

    // Scaling matrix
    mat4 scaleMatrix = {
        objScaleX,   0.0f,       0.0f,       0.0f,
        0.0f,        objScaleY,  0.0f,       0.0f,
        0.0f,        0.0f,       objScaleZ,  0.0f,
        0.0f,        0.0f,       0.0f,       1.0f
    };

    modelMatrix = translation * rotateMatrix * scaleMatrix;

    // Pass model matrix to cube.vert
    GLint location = glGetUniformLocation(shaderProgram, "modelMatrix");
    glUniformMatrix4fv(location, 1, GL_FALSE, &modelMatrix[0][0]);
}
