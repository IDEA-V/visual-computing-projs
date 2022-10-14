#pragma once

#include <string>

#include <glad/gl.h>
#include <glm/matrix.hpp>

#include "core/PluginRegister.h"
#include "core/RenderPlugin.h"

namespace OGL4Core2::Plugins::PCVC::HelloCube {

    class HelloCube : public Core::RenderPlugin {
        REGISTERPLUGIN(HelloCube, 101) // NOLINT

    public:
        static std::string name() {
            return "PCVC/HelloCube";
        }

        explicit HelloCube(const Core::Core& c);
        ~HelloCube() override;

        void render() override;
        void resize(int width, int height) override;
        void mouseMove(double xpos, double ypos) override;

    private:
        void renderGUI();

        void initBuffers();
        void initShaderProgram();
        void initTexture();

        void resetCube();
        void resetCamera();

        // Matrix updates
        void setProjectionMatrix();
        void setViewMatrix();
        void setModelMatrix();

        // Window state
        float aspectRatio;

        // GL objects
        GLuint va;
        GLuint vbo;
        GLuint ibo;
        GLuint shaderProgram;
        int numCubeFaceTriangles;
        GLuint texture;

        // Matrices
        glm::mat4 projectionMatrix;
        glm::mat4 viewMatrix;
        glm::mat4 modelMatrix;

        // Other variables
        // --------------------------------------------------------------------------------
        //  TODO: Define necessary variables.
        // --------------------------------------------------------------------------------

        // GUI parameters
        glm::vec3 backgroundColor;
        int mouseControlMode;
        int transformCubeMode;
        float cameraDolly;
        float cameraFoVy;
        float cameraPitch;
        float cameraYaw;
        float objTranslateX;
        float objTranslateY;
        float objTranslateZ;
        float objRotateX;
        float objRotateY;
        float objRotateZ;
        float objScaleX;
        float objScaleY;
        float objScaleZ;
        bool showWireframe;
        bool showTexture;
        int patternFreq;
    };

} // namespace OGL4Core2::Plugins::PCVC::HelloCube
