#include "PathTracing.h"

#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <fstream>
#include <utility>

#include <imgui.h>

#include "core/Core.h"
#include <glm/gtx/string_cast.hpp>
using namespace OGL4Core2;
using namespace OGL4Core2::Plugins::PCVC::PathTracing;
template<typename VertexDataType>
using VertexData = std::pair<std::vector<VertexDataType>, glowl::VertexLayout>;

/**
 * @brief PathTracing constructor.
 * Initlizes all variables with meaningful values and initializes
 * geometry, objects and shaders used in the scene.
 */
PathTracing::PathTracing(const Core::Core& c)
    : Core::RenderPlugin(c),
      wWidth(10),
      wHeight(10),
      newObjectType(0),
      currentObjectColor(glm::vec3(1.0f,1.0f,1.0f)),
      currentObjectRadius(1.0f),
      currentObjectSepcular(0.0f),
      currentObjectRoughness(0.0f),
      currentObjectMetalness(0.0f),
      frameNumber(0),
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
      oldViewMx(glm::mat4(1.0)),
      backgroundColor(glm::vec3(0.2f, 0.2f, 0.2f)),
      showDebug(false),
      showFBOAtt(0),
      fovY(45.0),
      zNear(0.01f),
      zFar(20.0f) {
    // Init Camera
    camera = std::make_shared<Core::OrbitCamera>(5.0f);
    core_.registerCamera(camera);

    initShaders();
    initVAs();
    glGenBuffers(1, &ObjectBuffer);

    //Light source
    // glm::vec3 pos = glm::vec3(-1.5f, -2.5f, 3.4f);
    glm::vec3 pos = glm::vec3(-0.75f, -1.5f, 3.4f);

    glm::vec4 color = glm::vec4(25.0f, 25.0f, 25.0f, 1.0f);
    // std::cout << glm::to_string(color) << std::endl;
    Object o = Object::LighSourceRect(1, pos, color, 1.0, 0.5, glm::vec3(1.5f, 0.0f, 0.0f), glm::vec3(0.0f, 1.5f, 0.0f));
    objectList.push_back(o);

    //Three Spheres
    pos = glm::vec3(-1.179999, -1.035008, 0.490001);
    color = glm::vec4(0.815, .418501512, .00180012, 1.0);
    o = Object::Sphere(2, pos, color, 3, 1.0, 0.4, 1.0, 1.0f);
    objectList.push_back(o);

    pos = glm::vec3(0.170000, 1.299999, 1.039999);
    color = glm::vec4(0.0, 0.0, 1.0, 1.0);
    o = Object::Sphere(3, pos, color, 2, 1.0, 0.1, 0.0, 1.0f);
    objectList.push_back(o);

    pos = glm::vec3(1.254999, -0.570000, -0.094999);
    color = glm::vec4(0.815, .418501512, .00180012, 1.0);
    o = Object::Sphere(4, pos, color, 2, 1.0, 0.4, 1.0, 1.0f);
    objectList.push_back(o);

    //Two Cube
    // pos = glm::vec3(-1.5, -0.2, -1.5);
    // color = glm::vec4(0.0, 0.0, 1.0, 1.0);
    // Object::Cube(3, pos, color, 2, 1.0, 0.5, 0.0, glm::vec3(-0.5f, 1.0f, 0.0f), glm::vec3(1.0f, 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 2.5f), objectList);
    // pos = glm::vec3(1.5, -2.0, -1.5);
    // color = glm::vec4(1.0, 0.0, 0.0, 1.0);
    // Object::Cube(3, pos, color, 2, 1.0, 0.5, 0.0, glm::vec3(-1.5f, 0.75f, 0.0f), glm::vec3(0.75f, 1.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.67f), objectList);


    //floor
    pos = glm::vec3(-2.5f, -2.5f, -1.5f);
    color = glm::vec4(1.0, 1.0, 1.0, 1.0);
    o = Object::Rect(0, pos, color, 1, 0.0, 0.0, 0.4, glm::vec3(5.0, 0.0, 0.0), glm::vec3(0.0, 5.0, 0.0));
    objectList.push_back(o);
    //ceiling
    pos = glm::vec3(-2.5f, -2.5f, 3.5f);
    color = glm::vec4(1.0, 1.0, 1.0, 1.0);
    o = Object::Rect(0, pos, color, 1, 0.0, 1.0, 0.0, glm::vec3(5.0, 0.0, 0.0), glm::vec3(0.0, 5.0, 0.0));
    objectList.push_back(o);
    //left wall
    pos = glm::vec3(-2.5f, -2.5f, -1.5f);
    color = glm::vec4(1.0, 0.1, 0.0, 1.0);
    o = Object::Rect(0, pos, color, 1, 0.0, 1.0, 0.0, glm::vec3(0.0, 5.0, 0.0), glm::vec3(0.0, 0.0, 5.0));
    objectList.push_back(o);
    //back wall
    pos = glm::vec3(-2.5f, 2.5f, -1.5f);
    color = glm::vec4(1.0, 1.0, 1.0, 1.0);
    o = Object::Rect(0, pos, color, 1, 0.0, 1.0, 0.0, glm::vec3(5.0, 0.0, 0.0)                                                                                                                                                , glm::vec3(0.0, 0.0, 5.0));
    objectList.push_back(o);
    //right wall
    pos = glm::vec3(2.5f, -2.5f, -1.5f);
    color = glm::vec4(0.1, 1.0, 0.0, 1.0);
    o = Object::Rect(0, pos, color, 1, 0.0, 1.0, 0.0, glm::vec3(0.0, 5.0, 0.0), glm::vec3(0.0, 0.0, 5.0));
    objectList.push_back(o);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ObjectBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Object) * objectList.size(), objectList.data(), GL_DYNAMIC_DRAW);


    // --------------------------------------------------------------------------------
    //  TODO: Load textures from the "resources/textures" folder.
    //        Use the "getTextureResource" helper function.
    // --------------------------------------------------------------------------------
    // Request some parameters
    GLint maxColAtt;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColAtt);
    std::cerr << "Maximum number of color attachments: " << maxColAtt << std::endl;

    GLint maxGeomOuputVerts;
    glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &maxGeomOuputVerts);
    std::cerr << "Maximum number of geometry output vertices: " << maxGeomOuputVerts << std::endl;

    // Initialize clear color and enable depth testing
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

/**
 * @brief PathTracing destructor.
 */
PathTracing::~PathTracing() {
    // --------------------------------------------------------------------------------
    //  Note: The glowl library uses the RAII principle. OpenGL objects are deleted in
    //        the destructor of the glowl wrapper objects. Therefore we must not delete
    //        them on our own. But keep this in mind and remember that this not always
    //        happens automatically.
    //
    //  TODO: Do not forget to clear all other allocated sources!
    // --------------------------------------------------------------------------------

    // Reset OpenGL state.
    glDeleteTextures(1, &fboTexColor);
    glDeleteTextures(1, &fboTexId);
    glDeleteTextures(1, &fboTexNormals);
    glDeleteTextures(1, &fboTexDepth);
    deleteFBOs();
    glDisable(GL_DEPTH_TEST);
}

/**
 * @brief Render GUI.
 */
void PathTracing::renderGUI() {
    if (ImGui::CollapsingHeader("PathTracing", ImGuiTreeNodeFlags_DefaultOpen)) {
        camera->drawGUI();
        // Dropdown menu to choose which framebuffer attachment to show
        ImGui::Combo("FBO attach.", &showFBOAtt, "Color\0IDs\0Normals\0LightView\0");

        // --------------------------------------------------------------------------------
        //  TODO: Setup ImGui elements for 'fovY', 'zNear', 'ZFar', 'lightLong', 'lightLat',
        //        'lightDist'. For the bonus task, you also need 'lightFoV'.
        // --------------------------------------------------------------------------------
        bool colorChanged = ImGui::ColorEdit3("color", reinterpret_cast<float*>(&currentObjectColor), ImGuiColorEditFlags_Float);
        if (colorChanged) {
            objectList[pickedObjNum].albedo = glm::vec4(currentObjectColor, 1.0f);
        }
        bool radiusChanged = ImGui::SliderFloat("radius", &currentObjectRadius, 0.0f, 1.0f);
        if (radiusChanged) {
            objectList[pickedObjNum].radius = currentObjectRadius;
        }
        bool specularChanged = ImGui::SliderFloat("specular", &currentObjectSepcular, 0.0f, 1.0f);
        if (specularChanged) {
            objectList[pickedObjNum].specular = currentObjectSepcular;
        }
        bool roughnessChanged = ImGui::SliderFloat("roughness", &currentObjectRoughness, 0.0f, 1.0f);
        if (roughnessChanged) {
            objectList[pickedObjNum].roughness = currentObjectRoughness;
        }
        bool metalnessChanged = ImGui::SliderFloat("metalness", &currentObjectMetalness, 0.0f, 1.0f);
        if (metalnessChanged) {
            objectList[pickedObjNum].metalness = currentObjectMetalness;
        }
        if (colorChanged || radiusChanged || specularChanged || roughnessChanged || metalnessChanged) {
            frameNumber = 0;
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, ObjectBuffer);
            glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Object) * objectList.size(), objectList.data(), GL_DYNAMIC_DRAW);
        }
        ImGui::Combo("Object Type", &newObjectType, "Sphere\0");
        if (ImGui::Button("Add")) {
            
            uint id=1;
            for (std::vector<Object>::reverse_iterator i = objectList.rbegin(); i != objectList.rend(); i++) {
                if (i->id != 0) {
                    id = i->id+1;
                    break;
                }
            }

            glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f);
            glm::vec4 color = glm::vec4(0.2, (float) rand()/RAND_MAX, (float) rand()/RAND_MAX, 1.0);
            // std::cout << glm::to_string(color) << std::endl;
            Object o = Object::Sphere(id, pos, color, 2, 1.0, 0.1, 0.0, 1.0f);
            objectList.push_back(o);
            frameNumber = 0;
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, ObjectBuffer);
            glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Object) * objectList.size(), objectList.data(), GL_DYNAMIC_DRAW);

            // glBindBuffer(GL_SHADER_STORAGE_BUFFER, ObjectBuffer);
            // Object obs[objectList.size()];
            // glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Object)*objectList.size(), obs);
            // std::cout << glm::to_string(obs[0].pos) << std::endl;
        }
        ImGui::SliderFloat("fovY", &fovY, 1.0f, 90.0f);
        ImGui::SliderFloat("zNear", &zNear, 0.01f, zFar);
        ImGui::SliderFloat("zFar", &zFar, zNear, 50.0f);
        if (ImGui::Button("Save")) {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
            glReadBuffer(GL_COLOR_ATTACHMENT0);
            float *data = new float[3*wWidth*wHeight];
            glReadPixels(0, 0, wWidth, wHeight, GL_RGB, GL_FLOAT, data);
            std::ofstream file("img.pgm");
            uint index = 0;
            file << "P2" << std::endl;
            file << wWidth << " " << wHeight << std::endl;
            file << "255" << std::endl;

            for (int i=0;i<wWidth*wHeight; i++){
                file << static_cast<int>(255*data[i*3]);
                for (int j=1;j<3; j++){
                    file << " " << static_cast<int>(255*data[i*3+j]);
                }
                file << std::endl;
            }
            file.close();
            delete data;
        }
        bool debugChanged = ImGui::Checkbox("debug", &showDebug);
        if (debugChanged) {
            std::cout << "Debug Mode: " << showDebug << std::endl;
            frameNumber = 0;
        }
    }
}

void PathTracing::updateFrameNumber() {
    bool all_equals = true;
    for (size_t i = 0; i < 4; i++)
    {
        glm::vec4 equals = glm::equal(oldViewMx[i], camera->viewMx()[i]);
        for (size_t j = 0; j < 4; j++)
        {
            all_equals = all_equals && equals[j];
        }
        
    }
    if (!all_equals) {
        // std::cout << "changed" << std::endl;
        frameNumber=1;
    }else{
        frameNumber++;
    }
    oldViewMx = camera->viewMx();
}

/**
 * @brief PathTracing render callback.
 */
void PathTracing::render() {
    renderGUI();

    // Update the matrices for current frame.
    updateMatrices();

    // --------------------------------------------------------------------------------
    //  TODO: First render pass to fill the FBOs. Call the the drawTo... method(s).
    // --------------------------------------------------------------------------------
    updateFrameNumber();
    drawToFBO();
    
    //drawToLightFBO();
    // --------------------------------------------------------------------------------
    //  TODO: In the second render pass, a window filling quad is drawn and the FBO
    //    textures are used for deferred shading.
    // --------------------------------------------------------------------------------
    glViewport(0, 0, wWidth, wHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glm::mat4 orthoMx = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f);

    shaderQuad->use();
    shaderQuad->setUniform("projMx", orthoMx);
    shaderQuad->setUniform("showFBOAtt", showFBOAtt);

    GLint unit = 0; // Color
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, fboTexColor);
    shaderQuad->setUniform("fboTexColor", unit);

    unit = 1; // ID
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, fboTexId);
    shaderQuad->setUniform("fboTexId", unit);

    unit = 2; // Normals
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, fboTexNormals);
    shaderQuad->setUniform("fboTexNormals", unit);

    unit = 3; // Light View
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, fboTexColor);
    shaderQuad->setUniform("lightFboTexColor", unit);
    
    vaQuad->draw();

    glUseProgram(0); // release shaderQuad
    glBindTexture(GL_TEXTURE_2D, 0);

    std::cout << "Sample Numer: " << frameNumber << "\r";

    // int sample_limit = 50;
    // if (frameNumber >= sample_limit) {
    //     frameNumber = sample_limit;
    // }
}

/**
 * @brief PathTracing resize callback.
 * @param width  The current width of the window
 * @param height The current height of the window
 */
void PathTracing::resize(int width, int height) {

    if (width > 0 && height > 0) {
        wWidth = width;
        wHeight = height;
        std::cout << wWidth << " " << wHeight << std::endl;
        aspect = static_cast<float>(wWidth) / static_cast<float>(wHeight);

        // Every time the window size changes, the size, of the fbo has to be adapted.
        frameNumber = 0;
        GLuint texs[4] = {fboTexColor, fboTexId, fboTexNormals, fboTexDepth};
        glDeleteTextures(4, texs);
        deleteFBOs();
        initFBOs();
    }
}

/**
 * @brief PathTracing keyboard callback.
 * @param key      Which key caused the event
 * @param action   Which key action was performed
 * @param mods     Which modifiers were active (e. g. shift)
 */
void PathTracing::keyboard(Core::Key key, Core::KeyAction action, [[maybe_unused]] Core::Mods mods) {
    if (action != Core::KeyAction::Press) {
        return;
    }

    if (key == Core::Key::R) {
    } else if (key >= Core::Key::Key1 && key <= Core::Key::Key7) {
        showFBOAtt = static_cast<int>(key) - static_cast<int>(Core::Key::Key1);
    }
}

struct Ray {
    glm::vec3 o; // origin of the ray
    glm::vec3 d; // direction of the ray
};

bool intersectSphere(Ray r, float radius, glm::vec3 pos, float tNear, float tFar) {
    r.o -= pos;
    float a = dot(r.d, r.d);
    float b = 2.0*dot(r.o, r.d);
    float c = dot(r.o, r.o) - radius * radius;

    float discriminant = b*b-4*a*c;

    if (discriminant > 0) {
        tNear = (-b - sqrt(discriminant))/2/a;
        tFar = (-b + sqrt(discriminant))/2/a;
        std::cout << tNear<< " " << tFar << std::endl;
        if (tNear > 0) {
            if (tNear < 0.0001) {
                tNear = tFar;
                glm::vec3 hitPos = r.o + r.d*tFar;
                std::cout << tNear << std::endl;
                return true;
            }
            glm::vec3 hitPos = r.o + r.d*tNear;
            glm::vec3 normal = normalize(hitPos-pos);
            std::cout << tNear << std::endl;
            return true;
        }else{
            if (length(r.o-pos)<radius) {
                tNear = tFar;
                std::cout << tNear << std::endl;
                return true;
            }
        }
    }

    return false;
}


bool intersectRect(Ray r, glm::vec3 p, glm::vec3 s1, glm::vec3 s2) {
    glm::vec3 n = cross(s1,s2);
    if (glm::dot(r.d, n) != 0) {
        float tNear = glm::dot((p-r.o), n)/glm::dot(r.d, n);
        if (tNear>0) {
            glm::vec3 hitPos = r.o + tNear*r.d;
            std::cout << tNear << "||" << glm::to_string(hitPos) << std::endl;
            if (glm::dot((hitPos-p), s1) >= 0 && glm::dot((hitPos-p), s1) <= length(s1)*length(s1) && glm::dot((hitPos-p), s2) >= 0 && glm::dot((hitPos-p), s2) <= length(s2)*length(s2)) {
                return true;
            };

        }
    }

    return false;
}

/**
 * @brief PathTracing mouse callback.
 * @param button   Which mouse button caused the event
 * @param action   Which mouse action was performed
 * @param mods     Which modifiers were active (e. g. shift)
 */
void PathTracing::mouseButton(Core::MouseButton button, Core::MouseButtonAction action, Core::Mods mods) {
    // --------------------------------------------------------------------------------
    //  TODO: Add mouse button functionality.
    // --------------------------------------------------------------------------------
    if (button == Core::MouseButton::Left) {

        // Ray r = {glm::vec3(-1.1162f, -2.35654f, -0.678621f), glm::vec3(0.196532f, 0.287829f, 0.937299f)};
        // std::cout << intersectSphere(r, 1.0f, glm::vec3(-1.009999, -2.250013,  0.310001), 0.0f, 0.0f) << std::endl;

        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        float data[4];
        glReadPixels(lastMouseX, wHeight - lastMouseY, 1, 1, GL_RGBA, GL_FLOAT, data);
        glm::vec3 o = glm::vec3(data[0], data[1], data[2]);
        std::cout << "color0: " << data[0] << ", " << data[1] << ", " << data[2] << ", " << data[3] << std::endl;

        // Ray r{glm::vec3(0.0, 0.0, 0.0), normalize(glm::vec3(-1.0, 0.0, 1.0))};
        // std::cout << "intersect: " << intersectSphere(r, 1.0, glm::vec3(0.0,0.0,0.0), 0.0, 0.0) << std::endl;

        // glReadBuffer(GL_COLOR_ATTACHMENT1);
        // glReadPixels(lastMouseX, wHeight - lastMouseY, 1, 1, GL_RGBA, GL_FLOAT, data);
        // std::cout << "color1: " << data[0] << ", " << data[1] << ", " << data[2] << ", " << data[3] << std::endl;

        // glReadBuffer(GL_COLOR_ATTACHMENT2);
        // glReadPixels(lastMouseX, wHeight - lastMouseY, 1, 1, GL_RGBA, GL_FLOAT, data);
        // glm::vec3 d = glm::vec3(data[0], data[1], data[2]);
        // std::cout << "color2: " << data[0] << ", " << data[1] << ", " << data[2] << ", " << data[3] << std::endl;

        // glReadBuffer(GL_COLOR_ATTACHMENT2);
        // glReadPixels(lastMouseX, wHeight - lastMouseY, 1, 1, GL_RGB, GL_FLOAT, data);
        // std::cout << data[0] << ", " << data[1] << ", " << data[2] << std::endl;

        // Ray ray{o,d};
        // std::cout << "Intersect: " << intersectSphere(ray, 1.0f, glm::vec3(0.0f,0.0f,0.0f), 0.0f, 0.0f) << std::endl;
    //     return;
    }
    if ((action == Core::MouseButtonAction::Press) && mods.onlyControl()) {
        // ctrl + Mouse left button: Pick object
        if (button == Core::MouseButton::Left) {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
            glReadBuffer(GL_COLOR_ATTACHMENT1);

            //read from ID attachment
            unsigned int id;
            glReadPixels(lastMouseX, wHeight - lastMouseY, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &id);

            // glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
            // glReadBuffer(GL_COLOR_ATTACHMENT2);
            // float data[3];
            // glReadPixels(lastMouseX, wHeight - lastMouseY, 1, 1, GL_RGB, GL_FLOAT, data);
            // std::cout << data[0] << " " << data[1] << " " << data[2] << std::endl;

            pickedObjNum = -1;

            if (id > 0) {
                for (int i=0; i< objectList.size(); i++) {
                    if (objectList[i].id == id){
                        std::cout << "Picked ID: " << objectList[i].id << std::endl; 
                        pickedObjNum = i;
                        currentObjectColor = objectList[pickedObjNum].albedo;
                        currentObjectRadius = objectList[pickedObjNum].radius;
                        currentObjectSepcular = objectList[pickedObjNum].specular;
                        currentObjectRoughness = objectList[pickedObjNum].roughness;
                        currentObjectMetalness = objectList[pickedObjNum].metalness;

                    }
                }
            }

            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        // ctrl + mouse right button: move object in xy-plane
        } else if ((pickedObjNum >= 0) && (button == Core::MouseButton::Right)) {
            moveMode = ObjectMoveMode::XY;
        // ctrl + mouse middle button: move object in z-plane
        } else if ((pickedObjNum >= 0) && (button == Core::MouseButton::Middle)) {
            moveMode = ObjectMoveMode::Z;
        }
    }else if (action == Core::MouseButtonAction::Release) {
        moveMode = ObjectMoveMode::None;
    }
}

/**
 * @brief PathTracing mouse move callback.
 * Called after the mouse was moved, coordinates are measured in screen coordinates but
 * relative to the top-left corner of the window.
 * @param xpos     The x position of the mouse cursor
 * @param ypos     The y position of the mouse cursor
 */
void PathTracing::mouseMove(double xpos, double ypos) {
    // --------------------------------------------------------------------------------
    //  TODO: Add mouse move functionality.
    // --------------------------------------------------------------------------------
    
    //compute moving offset 
    float moveX = xpos - lastMouseX;
    float moveY = ypos - lastMouseY;
    float speedXY = 0.005;
    float speedZ = 0.01;

    if (moveMode == ObjectMoveMode::XY) {
        // objectList[pickedObjNum]->modelMx = glm::translate(objectList[pickedObjNum]->modelMx, glm::vec3(moveX*speedXY, -moveY*speedXY, 0.0f));
        glm::vec3 pos = objectList[pickedObjNum].pos;
        pos += glm::vec3(moveX*speedXY, -moveY*speedXY, 0.0f);
        frameNumber = 0;
        objectList[pickedObjNum].pos = pos;
        std::cout << "Moved to: " << glm::to_string(pos) << std::endl;
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ObjectBuffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Object) * objectList.size(), objectList.data(), GL_DYNAMIC_DRAW);
    } else if (moveMode == ObjectMoveMode::Z) {
        // objectList[pickedObjNum]->modelMx = glm::translate(objectList[pickedObjNum]->modelMx, glm::vec3(0.0f, 0.0f, -moveY * speedXY));
        glm::vec3 pos = objectList[pickedObjNum].pos;
        pos += glm::vec3(0.0f, 0.0f, -moveY * speedXY);
        frameNumber = 0;
        objectList[pickedObjNum].pos = pos;
        std::cout << "Moved to: " << glm::to_string(pos) << std::endl;
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ObjectBuffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Object) * objectList.size(), objectList.data(), GL_DYNAMIC_DRAW);
    }

    lastMouseX = xpos;
    lastMouseY = ypos;
}

/**
 * @brief Init shaders for the window filling quad and the box that is drawn around picked objects.
 */
void PathTracing::initShaders() {
    try {
        shaderQuad = std::make_unique<glowl::GLSLProgram>(glowl::GLSLProgram::ShaderSourceList{
            {glowl::GLSLProgram::ShaderType::Vertex, getStringResource("shaders/quad.vert")},
            {glowl::GLSLProgram::ShaderType::Fragment, getStringResource("shaders/quad.frag")}});
    } catch (glowl::GLSLProgramException& e) {
        std::cerr << e.what() << std::endl;
    }

    try {
        shaderPathTracer = std::make_unique<glowl::GLSLProgram>(glowl::GLSLProgram::ShaderSourceList{
            {glowl::GLSLProgram::ShaderType::Vertex, getStringResource("shaders/pathTracer.vert")},
            {glowl::GLSLProgram::ShaderType::Fragment, getStringResource("shaders/pathTracer.frag")}});
    } catch (glowl::GLSLProgramException& e) {
        std::cerr << e.what() << std::endl;
    }
}

/**
 * @brief Init vertex arrays.
 */
void PathTracing::initVAs() {
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
}

/**
 * @brief Initialize all frame buffer objects (FBOs).
 */
void PathTracing::initFBOs() {
    if (wWidth <= 0 || wHeight <= 0) {
        return;
    }

    // --------------------------------------------------------------------------------
    //  TODO: Create a frame buffer object (FBO) for multiple render targets.
    //        Use the createFBOTexture method to initialize an empty texture.
    // --------------------------------------------------------------------------------
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create textures for three color attachments
    
    fboTexColor = createFBOTexture(wWidth, wHeight, GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_LINEAR); // Color
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexColor, 0);

    fboTexId = createFBOTexture(wWidth, wHeight, GL_R32I, GL_RED_INTEGER, GL_UNSIGNED_INT, GL_LINEAR); // ID
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, fboTexId, 0);

    // fboTexId = createFBOTexture(wWidth, wHeight, GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_LINEAR); // Normals
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, fboTexId, 0);

    fboTexNormals = createFBOTexture(wWidth, wHeight, GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_LINEAR); // Normals
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, fboTexNormals, 0);

    // Create textures for depth attachment
    fboTexDepth = createFBOTexture(wWidth, wHeight, GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_FLOAT, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fboTexDepth, 0);

    // Check whether the framebuffer is complpete
    checkFBOStatus();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/**
 * @brief Delete all framebuffer objects.
 */
void PathTracing::deleteFBOs() {
    glDeleteFramebuffers(1, &fbo);
}

/**
 * @brief Check status of bound framebuffer object (FBO).
 */
void PathTracing::checkFBOStatus() {
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
GLuint PathTracing::createFBOTexture(int width, int height, const GLenum internalFormat, const GLenum format,
    const GLenum type, GLint filter) {
    // --------------------------------------------------------------------------------
    //  TODO: Generate an empty 2D texture. Set min/mag filters. Set wrap mode in (s,t).
    // --------------------------------------------------------------------------------
    GLuint texId = 0;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glBindTexture(GL_TEXTURE_2D, 0);
    return texId;
}

/**
 * @brief Update the matrices.
 */
void PathTracing::updateMatrices() {
    projMx = glm::perspective(glm::radians(fovY), aspect, zNear, zFar);
}

/**
 * @brief Draw to framebuffer object.
 */
void PathTracing::drawToFBO() {
    if (!glIsFramebuffer(fbo)) {
        return;
    }

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

    //clear all color buffers and depth buffer
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //reset color attachment to background color leave the rest zero
    // glClearTexImage(fboTexColor, 0, GL_RGB, GL_FLOAT, glm::value_ptr(backgroundColor));

    GLenum drawBuffer[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, drawBuffer);

    glm::mat4 orthoMx = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f);

    shaderPathTracer->use();
    shaderPathTracer->setUniform("projMx", orthoMx);

    //parameters for deferred lighting
    shaderPathTracer->setUniform("zNear", zNear);
    shaderPathTracer->setUniform("zFar", zFar);
    shaderPathTracer->setUniform("wWidth", wWidth);
    shaderPathTracer->setUniform("wHeight", wHeight);

    shaderPathTracer->setUniform("invViewMx", inverse(camera->viewMx()));
    shaderPathTracer->setUniform("invViewProjMx", inverse(camera->viewMx()) * inverse(projMx));

    shaderPathTracer->setUniform("objectNumber", (uint) objectList.size());
    shaderPathTracer->setUniform("rand", (float) rand()/RAND_MAX);
    shaderPathTracer->setUniform("frameNumber", frameNumber);
    shaderPathTracer->setUniform("showDebug", showDebug);

    GLint unit = 0; // Color
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, fboTexColor);
    shaderPathTracer->setUniform("fboTexColor", unit);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ObjectBuffer);

    vaQuad->draw();
    glUseProgram(0);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    // glDisable(GL_FRAMEBUFFER_SRGB);
}
