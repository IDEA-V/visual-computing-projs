#pragma once

#include <memory>
#include <string>
#include <vector>

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glowl/glowl.h>

#include "core/PluginRegister.h"
#include "core/RenderPlugin.h"
#include "core/camera/OrbitCamera.h"

namespace OGL4Core2::Plugins::PCVC::PathTracing {

    struct Object {
        int type;
        uint id;
        int emitting;
        int material;
        float specular;
        float roughness;
        float metalness;
        alignas(16) glm::vec3 pos;
        alignas(16) glm::vec4 albedo;
        float radius;
        alignas(16) glm::vec3 s1;
        alignas(16) glm::vec3 s2;

        static Object Sphere(uint id, glm::vec3 pos, glm::vec4 albedo, int material, float specular, float roughness, float metalness, float radius) {
            Object o = {0, id, 0, material, specular, roughness, metalness, pos, albedo, radius};
            return o;
        }

        static Object Rect(uint id, glm::vec3 pos, glm::vec4 albedo, int material, float specular, float roughness, float metalness, glm::vec3 s1, glm::vec3 s2) {
            Object o = {1, id, 0, material, specular, roughness, metalness, pos, albedo, 0.0f, s1, s2};
            return o;
        }

        static void Cube(uint id, glm::vec3 pos0, glm::vec4 albedo, int material, float specular, float roughness, float metalness, glm::vec3 s1, glm::vec3 s2, glm::vec3 s3, std::vector<Object>& l) {
            //floor
            glm::vec3 pos = pos0;
            Object o = Object::Rect(id, pos, albedo, material, specular, roughness, metalness, s1, s2);
            l.push_back(o);
            //left wall
            o = Object::Rect(id, pos, albedo, material, specular, roughness, metalness, s3, s1);
            l.push_back(o);
            //front wall
            o = Object::Rect(id, pos, albedo, material, specular, roughness, metalness, s3, s2);
            l.push_back(o);
            //ceiling
            pos = pos0+s3;
            o = Object::Rect(id, pos, albedo, material, specular, roughness, metalness, s1, s2);
            l.push_back(o);
            //back wall
            pos = pos0+s1;
            o = Object::Rect(id, pos, albedo, material, specular, roughness, metalness, s3, s2);
            l.push_back(o);
            //right wall
            pos = pos0+s2;
            o = Object::Rect(id, pos, albedo, material, specular, roughness, metalness, s3, s1);
            l.push_back(o);
        }

        static Object LighSourceRect(uint id, glm::vec3 pos, glm::vec4 albedo, float roughness, float metalness, glm::vec3 s1, glm::vec3 s2) {
            Object o = {1, id, 1, 2, 1.0, roughness, metalness, pos, albedo, 0.0f, s1, s2};
            return o;
        }

    };

    class PathTracing : public Core::RenderPlugin {
        REGISTERPLUGIN(PathTracing, 102) // NOLINT

    public:
        static std::string name() {
            return "PCVC/PathTracing";
        }

        explicit PathTracing(const Core::Core& c);
        ~PathTracing() override;

        void render() override;
        void resize(int width, int height) override;
        void keyboard(Core::Key key, Core::KeyAction action, Core::Mods mods) override;
        void mouseButton(Core::MouseButton button, Core::MouseButtonAction action, Core::Mods mods) override;
        void mouseMove(double xpos, double ypos) override;

    private:
        enum class ObjectMoveMode {
            None = 0,
            XY,
            Z,
        };

        void renderGUI();

        void initShaders();

        void initVAs();

        void initFBOs();
        void deleteFBOs();
        void checkFBOStatus();
        GLuint createFBOTexture(int width, int height, GLenum internalFormat, GLenum format, GLenum type, GLint filter);

        void updateMatrices();
        void updateFrameNumber();

        void drawToFBO();

        // Window state
        int wWidth;              //!< width of the window
        int wHeight;             //!< height of the window
        float aspect;            //!< aspect ratio of window
        double lastMouseX;       //!< last mouse position x
        double lastMouseY;       //!< last mouse position y
        ObjectMoveMode moveMode; //!< current state of interaction
        int frameNumber;

        // View
        glm::mat4 projMx;                          //!< Camera's projection matrix
        std::shared_ptr<Core::OrbitCamera> camera; //!< Camera's view matrix

        // GL objects
        std::unique_ptr<glowl::GLSLProgram> shaderQuad; //!< shader for quad
        std::unique_ptr<glowl::GLSLProgram> shaderPathTracer; //!< shader for path tracer
        std::unique_ptr<glowl::Mesh> vaQuad;            //!< quad vertices

        GLuint fbo;           //!< handle for FBO
        GLuint fboTexColor;   //!< handle for color attachments
        GLuint fboTexId;      //!< handle for color attachments
        GLuint fboTexNormals; //!< handle for color attachments
        GLuint fboTexDepth;   //!< handle for depth buffer attachment

        GLuint ObjectBuffer;
        std::vector<Object> objectList;

        // object state
        int pickedObjNum; //!< currently picked object, "< 0" = no object picked
        glm::mat4 oldViewMx;

        // GUI variables
        int newObjectType;
        float currentObjectRadius;
        float currentObjectSepcular;
        float currentObjectRoughness;
        float currentObjectMetalness;

        glm::vec3 currentObjectColor; //!< current object color
        glm::vec3 backgroundColor; //!< background color
        bool showDebug;
        int showFBOAtt;            //!< selector to show the different fbo attachments
        float fovY;                //!< Camera's vertical field of view
        float zNear;               //!< near clipping plane
        float zFar;                //!< far clipping plane
    };

} // namespace OGL4Core2::Plugins::PCVC::PathTracing
