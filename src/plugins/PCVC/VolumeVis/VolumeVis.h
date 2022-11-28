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

namespace OGL4Core2::Plugins::PCVC::VolumeVis {

    class VolumeVis : public Core::RenderPlugin {
        REGISTERPLUGIN(VolumeVis, 103) // NOLINT

    public:
        static std::string name() {
            return "PCVC/VolumeVis";
        }

        explicit VolumeVis(const Core::Core& c);
        ~VolumeVis() override;

        void render() override;
        void resize(int width, int height) override;
        void keyboard(Core::Key key, Core::KeyAction action, Core::Mods mods) override;
        void mouseMove(double xpos, double ypos) override;

    private:
        enum class ViewMode { LineOfSight = 0, Mip = 1, Isosurface = 2, Volume = 3, Noise = 4 };

        void renderGUI();

        void initShaders();

        void initVAs();

        void loadVolumeFile(int idx);
        void genHistogram(std::size_t bins, const std::vector<std::uint8_t>& values);

        void initTransferFunc();
        void updateTransferFunc(int channel, float value);
        void updateTransferFunc(int idx, int channel, float value);
        void loadTransferFunc(const std::string& filename);
        void saveTransferFunc(const std::string& filename);

        int wWidth;  //!< window width
        int wHeight; //!< window height

        std::vector<std::filesystem::path> datFiles;
        std::string datFilesGuiString;
        int currentFileLoaded;
        int currentFileSelection;

        glm::uvec3 volumeRes;
        glm::vec3 volumeDim;

        std::shared_ptr<Core::OrbitCamera> camera; //!< camera
        float fovY;                                //!< camera's vertical field of view
        glm::vec3 backgroundColor;

        bool useLinearFilter; //!< toggle linear texture filtering
        bool showBox;         //!< toggle box drawing
        ViewMode viewMode;

        int maxSteps;   //!< Maximum number of integration steps
        float stepSize; //!< Step size
        float scale;    //!< Global scaling factor

        float isoValue;
        glm::vec3 ambientColor;
        glm::vec3 diffuseColor;
        glm::vec3 specularColor;
        float k_ambient;
        float k_diffuse;
        float k_specular;
        float k_exp;

        std::size_t tfNumPoints;   //!< number of point for transfer functions
        std::vector<float> tfData; //!< transfer function values (r,g,b,a)

        int editorHeight;       //!< Height of the colormap editor/histogram panel
        int colormapHeight;     //!< Height of the colormap preview panel
        bool histoLogplot;      //!< toggle logplot
        bool useRandom;         //!< toggle random offset
        int tfChannel;          //!< TF channel enumeration: r,g,b,a
        std::string tfFilename; //!< TF filename for loading and saving

        std::size_t histoNumBins;  //!< number of bins for histogram
        uint32_t histoMaxBinValue; //!< maximum bin value

        std::unique_ptr<glowl::GLSLProgram> shaderVolume;     //!< shader program for volume rendering
        std::unique_ptr<glowl::GLSLProgram> shaderBackground; //!< shader program for box rendering
        std::unique_ptr<glowl::GLSLProgram> shaderHisto;      //!< shader program for histogram rendering
        std::unique_ptr<glowl::GLSLProgram> shaderTfLines;    //!< shader program for histogram background
        std::unique_ptr<glowl::GLSLProgram> shaderTfView;     //!< shader program for transfer functions

        std::unique_ptr<glowl::Mesh> vaQuad;         //!< vertex array for histogram data
        std::unique_ptr<glowl::Mesh> vaHisto;        //!< vertex array for histogram data
        std::unique_ptr<glowl::Mesh> vaTransferFunc; //!< vertex array for transfer functions

        GLuint volumeTex; //!< texture handle for volume data
        GLuint tfTex;     //!< transfer function texture handle
    };
} // namespace OGL4Core2::Plugins::PCVC::VolumeVis
