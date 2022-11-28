#pragma once

#include <memory>
#include <string>
#include <vector>

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glowl/glowl.h>

#include "core/camera/OrbitCamera.h"
#include "core/PluginRegister.h"
#include "core/RenderPlugin.h"

namespace OGL4Core2::Plugins::PCVC::SurfaceVis {

    class SurfaceVis : public Core::RenderPlugin {
        REGISTERPLUGIN(SurfaceVis, 104) // NOLINT

    public:
        static std::string name() {
            return "PCVC/SurfaceVis";
        }

        explicit SurfaceVis(const Core::Core& c);
        ~SurfaceVis() override;

        void render() override;
        void resize(int width, int height) override;
        void keyboard(Core::Key key, Core::KeyAction action, Core::Mods mods) override;
        void mouseButton(Core::MouseButton button, Core::MouseButtonAction action, Core::Mods mods) override;
        void mouseMove(double xpos, double ypos) override;

    private:
        static glm::vec3 idToColor(unsigned int id);
        static unsigned int colorToId(const unsigned char col[3]);

        void renderGUI();

        void initShaders();
        void initVAs();

        void initFBO();
        void drawToFBO();

        void loadControlPoints(const std::string& filename);
        void saveControlPoints(const std::string& filename);

        // --------------------------------------------------------------------------------
        //  TODO: Define methods needed for surface/control point handling.
        // --------------------------------------------------------------------------------

        // Window state
        int wWidth;        //!< window width
        int wHeight;       //!< window height
        double lastMouseX; //!< last mouse position x
        double lastMouseY; //!< last mouse position y

        // View
        std::shared_ptr<Core::OrbitCamera> camera; //!< view matrix
        glm::mat4 projMx;                          //!< projection matrix

        // GL objects
        std::unique_ptr<glowl::GLSLProgram> shaderQuad;           //!< shader program for window filling rectangle
        std::unique_ptr<glowl::GLSLProgram> shaderBox;            //!< shader program for box rendering
        std::unique_ptr<glowl::GLSLProgram> shaderControlPoints;  //!< shader program for control point rendering
        std::unique_ptr<glowl::GLSLProgram> shaderBSplineSurface; //!< shader program for b-spline surface rendering

        std::unique_ptr<glowl::Mesh> vaQuad;          //!< vertex array for window filling rectangle
        std::unique_ptr<glowl::Mesh> vaBox;           //!< vertex array for box
        std::unique_ptr<glowl::Mesh> vaControlPoints; //!< vertex array for control points
        GLuint vaEmpty;                               //!< vertex array for b-spline, not necessary

        std::unique_ptr<glowl::FramebufferObject> fbo;

        // Other variables
        int maxTessGenLevel;
        int degree_p;
        int degree_q;

        // --------------------------------------------------------------------------------
        //  TODO: Define variables needed for surface generation/state.
        // --------------------------------------------------------------------------------

        // GUI variables
        float fovY;               //!< camera's vertical field of view
        bool showBox;             //!< toggle box drawing
        bool showNormals;         //!< toggle show normals
        bool useWireframe;        //!< toggle wireframe drawing
        int showControlPoints;    //!< toggle control point drawing
        float pointSize;          //!< point size of control points
        std::string dataFilename; //!< Filename for loading/storing data

        // --------------------------------------------------------------------------------
        //  TODO: Define GUI variables needed for surface:
        //    number of control points, picked control point, position, tessellation level
        // --------------------------------------------------------------------------------

        glm::vec3 ambientColor;
        glm::vec3 diffuseColor;
        glm::vec3 specularColor;
        float k_ambient;
        float k_diffuse;
        float k_specular;
        float k_exp;
        int freq; //!< frequency of checkerboard texture
    };
} // namespace OGL4Core2::Plugins::PCVC::SurfaceVis
