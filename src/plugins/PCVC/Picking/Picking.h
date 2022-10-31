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

namespace OGL4Core2::Plugins::PCVC::Picking {
    class Object;

    class Picking : public Core::RenderPlugin {
        REGISTERPLUGIN(Picking, 102) // NOLINT

    public:
        static std::string name() {
            return "PCVC/Picking";
        }

        explicit Picking(const Core::Core& c);
        ~Picking() override;

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

        void drawToFBO();

        void drawToLightFBO();


        // Window state
        int wWidth;              //!< width of the window
        int wHeight;             //!< height of the window
        float aspect;            //!< aspect ratio of window
        double lastMouseX;       //!< last mouse position x
        double lastMouseY;       //!< last mouse position y
        ObjectMoveMode moveMode; //!< current state of interaction

        // View
        glm::mat4 projMx;                          //!< Camera's projection matrix
        std::shared_ptr<Core::OrbitCamera> camera; //!< Camera's view matrix

        // GL objects
        std::unique_ptr<glowl::GLSLProgram> shaderBox;  //!< shader for box
        std::unique_ptr<glowl::GLSLProgram> shaderQuad; //!< shader for quad
        std::unique_ptr<glowl::Mesh> vaBox;             //!< box vertices
        std::unique_ptr<glowl::Mesh> vaQuad;            //!< quad vertices

        GLuint fbo;           //!< handle for FBO
        GLuint fboTexColor;   //!< handle for color attachments
        GLuint fboTexId;      //!< handle for color attachments
        GLuint fboTexNormals; //!< handle for color attachments
        GLuint fboTexDepth;   //!< handle for depth buffer attachment

        std::shared_ptr<glowl::Texture2D> texEarth; //!< earth texture
        std::shared_ptr<glowl::Texture2D> texDice;  //!< dice texture
        std::shared_ptr<glowl::Texture2D> texBoard; //!< board texture

        // object state
        int pickedObjNum; //!< currently picked object, "< 0" = no object picked
        std::vector<std::shared_ptr<Object>> objectList;

        // light view (bonus task only)
        float lightZnear;      //!< near clipping plane of spot light
        float lightZfar;       //!< far clipping plane of spot light
        glm::mat4 lightProjMx; //!< spot light's projection matrix
        glm::mat4 lightViewMx; //!< spot light's view matrix

        // light fbo (bonus task only)
        int lightFboWidth;       //!< width of FBO
        int lightFboHeight;      //!< height of FBO
        GLuint lightFbo;         //!< handle for FBO
        GLuint lightFboTexColor; //!< handle for color attachment
        GLuint lightFboTexDepth; //!< handle for depth buffer attachment

        // GUI variables
        glm::vec3 backgroundColor; //!< background color
        bool useWireframe;         //!< toggle wireframe mode
        int showFBOAtt;            //!< selector to show the different fbo attachments
        float fovY;                //!< Camera's vertical field of view
        float zNear;               //!< near clipping plane
        float zFar;                //!< far clipping plane
        float lightLong;           //!< position of light, longitude in degree
        float lightLat;            //!< position of light, latitude in degree
        float lightDist;           //!< position of light, distance from origin
        float lightFoV;            //!< field of view of spot light (bonus task only)
    };

} // namespace OGL4Core2::Plugins::PCVC::Picking
