#include "VolumeVis.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>

#include <datraw.h>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <glm/gtx/string_cast.hpp>
#include <string>
#include <tuple>
#include <iostream>

#include "core/Core.h"
#include "core/util/ImGuiUtil.h"

using namespace OGL4Core2;
using namespace OGL4Core2::Plugins::PCVC::VolumeVis;

/**
 * @brief VolumeVis constructor.
 */
VolumeVis::VolumeVis(const Core::Core& c)
    : Core::RenderPlugin(c),
      wWidth(32),
      wHeight(32),
      currentFileLoaded(0),
      currentFileSelection(0),
      volumeRes(glm::uvec3(0)),
      volumeDim(glm::vec3(0.0)),
      fovY(45.0f),
      backgroundColor(glm::vec3(0.2f, 0.2f, 0.2f)),
      useLinearFilter(true),
      showBox(true),
      viewMode(ViewMode::LineOfSight),
      // --------------------------------------------------------------------------------
      // TODO: Set maxSteps to reasonable default, explain here! Current value is just a placeholder.
      // --------------------------------------------------------------------------------
      maxSteps(600),
      stepSize(0.001f),
      scale(0.02f),
      isoValue(0.5f),
      ambientColor(glm::vec3(1.0f, 1.0f, 1.0f)),
      diffuseColor(glm::vec3(1.0f, 1.0f, 1.0f)),
      specularColor(glm::vec3(1.0f, 1.0f, 1.0f)),
      k_ambient(0.2f),
      k_diffuse(0.7f),
      k_specular(0.1f),
      k_exp(120.0f),
      tfNumPoints(256),
      editorHeight(200),
      colormapHeight(20),
      histoLogplot(false),
      useRandom(true),
      tfChannel(0),
      tfFilename("test.tf"),
      histoNumBins(256),
      histoMaxBinValue(0),
      volumeTex(0),
      tfTex(0) {
    // Init Camera
    camera = std::make_shared<Core::OrbitCamera>(2.0f);
    core_.registerCamera(camera);

    // Load list of data files.
    datFiles = getResourceDirFilePaths("volumes", "^.*\\.dat$");
    datFilesGuiString.clear();
    for (const auto& file : datFiles) {
        datFilesGuiString += file.stem().string() + '\0';
    }
    datFilesGuiString += '\0';

    // Initialize shaders and vertex arrays
    initShaders();
    initVAs();

    // Load the volume file and its transfer function
    loadVolumeFile(0);
    loadTransferFunc("engine.tf");
    initTransferFunc();

    // Set OpenGL state.
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

/**
 * @brief VolumeVis destructor: Cleans up textures.
 */
VolumeVis::~VolumeVis() {
    // --------------------------------------------------------------------------------
    //  TODO: Do not forget to clear all allocated sources.
    // --------------------------------------------------------------------------------
    glDeleteTextures(1, &volumeTex);
    glDeleteTextures(1, &tfTex);
    // Reset OpenGL state.
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_BLEND);
}

/**
 * @brief Render GUI.
 */
void VolumeVis::renderGUI() {
    if (ImGui::CollapsingHeader("VolumeVis", ImGuiTreeNodeFlags_DefaultOpen)) {
        camera->drawGUI();

        ImGui::SliderFloat("FoVy", &fovY, 5.0f, 90.0f);
        ImGui::ColorEdit3("Background Color", reinterpret_cast<float*>(&backgroundColor), ImGuiColorEditFlags_Float);
        ImGui::Combo("Volume", &currentFileSelection, datFilesGuiString.c_str());
        // Show the resolution of the volume
        ImGui::Text("ResX: %i", volumeRes.x);
        ImGui::Text("ResY: %i", volumeRes.y);
        ImGui::Text("ResZ: %i", volumeRes.z);
        // Whether to use linear filtering
        ImGui::Checkbox("Lin. Filter", &useLinearFilter);
        ImGui::Checkbox("ShowBox", &showBox);
        Core::ImGuiUtil::EnumCombo("Mode", viewMode,
            {
                {ViewMode::LineOfSight, "LineOfSight"},
                {ViewMode::Mip, "Mip"},
                {ViewMode::Isosurface, "Isosurface"},
                {ViewMode::Volume, "Volume"},
                {ViewMode::Noise, "Noise"},
            });
        ImGui::InputInt("MaxSteps", &maxSteps);
        maxSteps = std::clamp(maxSteps, 1, 10000);
        ImGui::InputFloat("StepSize", &stepSize, 0.005f);
        stepSize = std::clamp(stepSize, 0.0f, 1.0f);
        if (viewMode != ViewMode::Isosurface) {
            ImGui::InputFloat("Scale", &scale, 0.1f);
        }
        if (viewMode == ViewMode::Isosurface) {
            ImGui::InputFloat("IsoValue", &isoValue, 0.01f);
            isoValue = std::clamp(isoValue, 0.0f, 100.0f);
            ImGui::ColorEdit3("Ambient", reinterpret_cast<float*>(&ambientColor), ImGuiColorEditFlags_Float);
            ImGui::ColorEdit3("Diffuse", reinterpret_cast<float*>(&diffuseColor), ImGuiColorEditFlags_Float);
            ImGui::ColorEdit3("Specular", reinterpret_cast<float*>(&specularColor), ImGuiColorEditFlags_Float);
            ImGui::SliderFloat("k_amb", &k_ambient, 0.0f, 1.0f);
            ImGui::SliderFloat("k_diff", &k_diffuse, 0.0f, 1.0f);
            ImGui::SliderFloat("k_spec", &k_specular, 0.0f, 1.0f);
            ImGui::SliderFloat("k_exp", &k_exp, 0.0f, 5000.0f);
        }
        if (viewMode == ViewMode::Volume) {
            ImGui::SliderInt("editor height", &editorHeight, 0, 500);
            ImGui::Checkbox("LogPlot", &histoLogplot);
            ImGui::Checkbox("random offset", &useRandom);
            ImGui::Combo("TF channel", &tfChannel, "red\0green\0blue\0alpha\0");
            ImGui::InputText("TF filename", &tfFilename);
        }
    }
    // ImGui::Combo also returns true if the same entry is selected again.
    // Only load data if value really changed.
    if (currentFileSelection != currentFileLoaded) {
        loadVolumeFile(currentFileSelection);
    }
}

/**
 * @brief VolumeVis render callback.
 */
void VolumeVis::render() {
    renderGUI();

    glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float viewAspect = 1.0f;
    if (viewMode == ViewMode::Volume) {
        // --------------------------------------------------------------------------------
        //  TODO: Set the viewport and viewAspect.
        // --------------------------------------------------------------------------------
        float volumeWHeight = wHeight - editorHeight;
        glViewport(0, editorHeight, wWidth, volumeWHeight);
        viewAspect = static_cast<float>(wWidth) / static_cast<float>(volumeWHeight);
    } else {
        glViewport(0, 0, wWidth, wHeight);
        viewAspect = static_cast<float>(wWidth) / static_cast<float>(wHeight);
    }
    glm::mat4 orthoProjMx = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f);

    // --------------------------------------------------------------------------------
    //  TODO: Draw (only) the volume.
    // --------------------------------------------------------------------------------
    shaderVolume->use();


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, volumeTex);
    shaderVolume->setUniform("volumeTex", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, tfTex);
    shaderVolume->setUniform("transferTex", 1);

    glm::mat4 projMx = glm::perspective(glm::radians(fovY), viewAspect, 1.0f, 50.0f);
    shaderVolume->setUniform("orthoProjMx", orthoProjMx);
    shaderVolume->setUniform("invViewMx", inverse(camera->viewMx()));
    shaderVolume->setUniform("invViewProjMx", inverse(camera->viewMx()) * inverse(projMx));

    shaderVolume->setUniform("volumeRes", (glm::vec3)volumeRes);
    shaderVolume->setUniform("volumeDim", volumeDim);

    shaderVolume->setUniform("viewMode", (int)viewMode);
    shaderVolume->setUniform("showBox", showBox);
    shaderVolume->setUniform("useRandom", useRandom);
    

    shaderVolume->setUniform("maxSteps", maxSteps);
    shaderVolume->setUniform("stepSize", stepSize);
    shaderVolume->setUniform("scale", scale);
    
    shaderVolume->setUniform("isovalue", isoValue);

    shaderVolume->setUniform("ambient", ambientColor);
    shaderVolume->setUniform("diffuse", diffuseColor);
    shaderVolume->setUniform("specular", specularColor);
    shaderVolume->setUniform("k_amb", k_ambient);
    shaderVolume->setUniform("k_diff", k_diffuse);
    shaderVolume->setUniform("k_spec", k_specular);
    shaderVolume->setUniform("k_exp", k_exp);
    
    shaderVolume->setUniform("width", wWidth);
    shaderVolume->setUniform("height", wHeight);
    // int t = static_cast<int> (time(NULL));

    vaQuad->draw();
    glUseProgram(0);
    glBindTexture(GL_TEXTURE_3D, 0);

    if (viewMode == ViewMode::Volume) {
        // --------------------------------------------------------------------------------
        //  TODO: Draw the transfer-function editor and histogram.
        // --------------------------------------------------------------------------------
        glDisable(GL_DEPTH_TEST);
        glViewport(0, 0, wWidth, editorHeight);
        viewAspect = static_cast<float>(wWidth) / static_cast<float>(wHeight / 4);

        // Draw checkerbroad background
        shaderBackground->use();
        shaderBackground->setUniform("orthoProjMx", orthoProjMx);
        shaderBackground->setUniform("aspect", viewAspect);
        vaQuad->draw();
        glUseProgram(0);

        // Draw histogram
        shaderHisto->use();
        shaderHisto->setUniform("maxBinValue", (float)histoMaxBinValue);
        shaderHisto->setUniform("logPlot", histoLogplot);
        shaderHisto->setUniform("orthoProjMx", orthoProjMx);
        float binStepHalf = 1.0f / (histoNumBins-1);
        shaderHisto->setUniform("binStepHalf", binStepHalf);

        vaHisto->draw();
        glUseProgram(0);
        glEnable(GL_DEPTH_TEST);

        shaderTfLines->use();
        shaderTfLines->setUniform("orthoProjMx", orthoProjMx);
        shaderTfLines->setUniform("channel", tfChannel);
        vaTransferFunc->draw();
        glUseProgram(0);

        shaderTfView->use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_1D, tfTex);
        shaderTfView->setUniform("tex", 0);
        shaderTfView->setUniform("orthoProjMx", orthoProjMx);
        shaderTfView->setUniform("aspect", viewAspect);
        vaQuad->draw();
        glUseProgram(0);

    }
}

/**
 * @brief VolumeVis resize callback.
 * @param width  The current width of the window
 * @param height The current height of the window
 */
void VolumeVis::resize(int width, int height) {
    if (width > 0 && height > 0) {
        wWidth = width;
        wHeight = height;
    }
}

/**
 * @brief VolumeVis keyboard callback.
 * @param key      Which key caused the event
 * @param action   Which key action was performed
 * @param mods     Which modifiers were active (e. g. shift)
 */
void VolumeVis::keyboard(Core::Key key, Core::KeyAction action, [[maybe_unused]] Core::Mods mods) {
    if (action != Core::KeyAction::Press) {
        return;
    }

    switch (key) {
        case Core::Key::F5: {
            // reload shaders
            initShaders();
            break;
        }
        case Core::Key::Key1: {
            viewMode = ViewMode::LineOfSight;
            break;
        }
        case Core::Key::Key2: {
            viewMode = ViewMode::Mip;
            break;
        }
        case Core::Key::Key3: {
            viewMode = ViewMode::Isosurface;
            break;
        }
        case Core::Key::Key4: {
            viewMode = ViewMode::Volume;
            break;
        }
        case Core::Key::R: {
            // subsequent modifications are performed on the red channel
            tfChannel = 0;
            break;
        }
        case Core::Key::G: {
            // subsequent modifications are performed on the green channel
            tfChannel = 1;
            break;
        }
        case Core::Key::B: {
            // subsequent modifications are performed on the blue channel
            tfChannel = 2;
            break;
        }
        case Core::Key::A: {
            // subsequent modifications are performed on the alpha channel
            tfChannel = 3;
            break;
        }
        case Core::Key::L: {
            if (viewMode == ViewMode::Volume) {
                loadTransferFunc(tfFilename);
            }
            break;
        }
        case Core::Key::S: {
            if (viewMode == ViewMode::Volume) {
                saveTransferFunc(tfFilename);
            }
            break;
        }
        default:
            break;
    }

    // --------------------------------------------------------------------------------
    //  TODO: Add keyboard functionality for transfer function editor.
    // --------------------------------------------------------------------------------
    if (key == Core::Key::Up) {
        updateTransferFunc(tfChannel, 1.0f);
    }
    if (key == Core::Key::Down) {
        updateTransferFunc(tfChannel, 0.0f);        
    }
}

/**
 * @brief VolumeVis mouse move callback.
 * Called after the mouse was moved, coordinates are measured in screen coordinates but
 * relative to the top-left corner of the window.
 * @param xpos     The x position of the mouse cursor
 * @param ypos     The y position of the mouse cursor
 */
void VolumeVis::mouseMove(double xpos, double ypos) {
    // --------------------------------------------------------------------------------
    //  TODO: Implement editing of transfer function.
    // --------------------------------------------------------------------------------
    if (this->core_.isMouseButtonPressed(Core::MouseButton::Left) && this->core_.isKeyPressed(Core::Key::LeftControl)) {
        int x = round(xpos/wWidth*255);
        float y = ((wHeight - ypos)/editorHeight-0.1)/0.9;

        if (x>255) x = 255;
        if (y > 1.0) y = 1.0;
        if (y < 0.0) y = 0.0;

        // std::cout << x << " " << y << std::endl;
        updateTransferFunc(x, tfChannel, y);

    }
}

/**
 * @brief Initialize shaders.
 */
void VolumeVis::initShaders() {
    // Initialize shader for volume
    try {
        shaderVolume = std::make_unique<glowl::GLSLProgram>(glowl::GLSLProgram::ShaderSourceList{
            {glowl::GLSLProgram::ShaderType::Vertex, getStringResource("shaders/volume.vert")},
            {glowl::GLSLProgram::ShaderType::Fragment, getStringResource("shaders/volume.frag")}});
    } catch (glowl::GLSLProgramException& e) {
        std::cerr << e.what() << std::endl;
    }

    // Initialize shader for background
    try {
        shaderBackground = std::make_unique<glowl::GLSLProgram>(glowl::GLSLProgram::ShaderSourceList{
            {glowl::GLSLProgram::ShaderType::Vertex, getStringResource("shaders/background.vert")},
            {glowl::GLSLProgram::ShaderType::Fragment, getStringResource("shaders/background.frag")}});
    } catch (glowl::GLSLProgramException& e) {
        std::cerr << e.what() << std::endl;
    }

    // Initialize shader for histogram
    try {
        shaderHisto = std::make_unique<glowl::GLSLProgram>(glowl::GLSLProgram::ShaderSourceList{
            {glowl::GLSLProgram::ShaderType::Vertex, getStringResource("shaders/histo.vert")},
            {glowl::GLSLProgram::ShaderType::Geometry, getStringResource("shaders/histo.geom")},
            {glowl::GLSLProgram::ShaderType::Fragment, getStringResource("shaders/histo.frag")}});
    } catch (glowl::GLSLProgramException& e) {
        std::cerr << e.what() << std::endl;
    }

    // Initialize shader for transfer function lines
    try {
        shaderTfLines = std::make_unique<glowl::GLSLProgram>(glowl::GLSLProgram::ShaderSourceList{
            {glowl::GLSLProgram::ShaderType::Vertex, getStringResource("shaders/tf-lines.vert")},
            {glowl::GLSLProgram::ShaderType::Fragment, getStringResource("shaders/tf-lines.frag")}});
    } catch (glowl::GLSLProgramException& e) {
        std::cerr << e.what() << std::endl;
    }

    // Initialize shader for transfer function preview
    try {
        shaderTfView = std::make_unique<glowl::GLSLProgram>(glowl::GLSLProgram::ShaderSourceList{
            {glowl::GLSLProgram::ShaderType::Vertex, getStringResource("shaders/tf-view.vert")},
            {glowl::GLSLProgram::ShaderType::Fragment, getStringResource("shaders/tf-view.frag")}});
    } catch (glowl::GLSLProgramException& e) {
        std::cerr << e.what() << std::endl;
    }
}

/**
 * @brief Init vertex arrays.
 */
void VolumeVis::initVAs() {
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
    vaQuad = std::make_unique<glowl::Mesh>(vertexDataQuad, quadIndices, GL_UNSIGNED_INT, GL_TRIANGLE_STRIP, GL_STATIC_DRAW);
}

/**
 * @brief Load volume file.
 * @param idx   The file index
 */
void VolumeVis::loadVolumeFile(int idx) {
    if (idx < 0 || idx >= static_cast<int>(datFiles.size())) {
        throw std::runtime_error("Invalid file index!");
    }
    currentFileLoaded = idx;

    std::string volumeFile = datFiles[idx].string();

    // --------------------------------------------------------------------------------
    //  TODO: Read data from 'volumeFile' using datraw::raw_reader<char>. Use slice
    //        thickness to determine correct volume dimensions. Normalize dimensions
    //        such that the maximum dimension is 1.0.
    //        Calculate the histogram. Upload the volume as a 3D texture.
    // --------------------------------------------------------------------------------

    datraw::raw_reader<char> rd = datraw::raw_reader<char>::open(volumeFile);
    std::vector<datraw::uint8> raw = rd.read_current();
    volumeRes = glm::uvec3(rd.info().resolution()[0], rd.info().resolution()[1], rd.info().resolution()[2]);
    // std::cout << glm::to_string(volumeRes) << std::endl;
    float max = std::max(std::max(volumeRes.x, volumeRes.y), volumeRes.z);
    volumeDim = glm::vec3(volumeRes.x / max, volumeRes.y / max, volumeRes.z / max);
    // std::cout << glm::to_string(volumeDim) << std::endl;

    glGenTextures(1, &volumeTex);
    glBindTexture(GL_TEXTURE_3D, volumeTex);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, rd.info().resolution()[0], rd.info().resolution()[1], rd.info().resolution()[2], 0, GL_RED, GL_UNSIGNED_BYTE, &raw[0]);
    glBindTexture(GL_TEXTURE_3D, 0);

    genHistogram(histoNumBins, raw);
}

/**
 * @brief Create the histogram.
 * @param bins     The number of bins
 * @param values   The vector with the histogram values
 */
void VolumeVis::genHistogram(std::size_t bins, const std::vector<std::uint8_t>& values) {
    if (bins == 0 || values.empty()) {
        return;
    }
    // --------------------------------------------------------------------------------
    //  TODO: Calculate the histogram and init the vertex array. Values are uint8,
    //        therefore the value range is [0, 255].
    //        Divide this value range into "bins" number of bins.
    // --------------------------------------------------------------------------------
    float* histogramValueArray = new float[bins]{0};
    for (int i = 0; i < values.size(); i++) {
        if (bins <=256) {
            histogramValueArray[(int)round(values[i]/255.0f*(bins-1))] += 1.0f;
        }else{
            uint v = values[i];
            int a = ceil( (bins-1)/255.0f*(v-0.5));
            int b = floor((bins-1)/255.0f*(v+0.5));
            for (int j = a; j <= b; j++)
            {
                if (j >=0 && j<bins){
                    histogramValueArray[j] += 1.0f;
                }
            }
            
        }
    }

    // Create vertex array and indices
    std::vector<float> histogramVertices;
    std::vector<GLuint> histogramIndices;
    for (int i = 0; i < bins; i++) {
        histogramVertices.push_back(i / (float)(bins-1));
        histogramVertices.push_back(histogramValueArray[i]);
        // std::cout << i / (float)(bins-1) << ' '<< histogramValueArray[i] << std::endl;
        histogramIndices.push_back(i);
        if (histoMaxBinValue < histogramValueArray[i]) histoMaxBinValue = histogramValueArray[i];
    }

    glowl::Mesh::VertexDataList<float> histoData{
        {histogramVertices, {8, {{2, GL_FLOAT, GL_FALSE, 0}}}},
    };

    vaHisto = std::make_unique<glowl::Mesh>(histoData, histogramIndices, GL_UNSIGNED_INT, GL_POINTS, GL_STATIC_DRAW);

    delete[] histogramValueArray;
}

/**
 * @brief Initialize the transfer function.
 */
void VolumeVis::initTransferFunc() {
    // --------------------------------------------------------------------------------
    //  TODO: Initialize the transfer function vertex array and load the transfer
    //        function data into a 1D texture.
    // --------------------------------------------------------------------------------

    std::vector<float> tfLocations;
    std::vector<GLuint> tfIndices;
    for (int i = 0; i < histoNumBins; i++) {
        tfLocations.push_back(i / (float)(histoNumBins-1));
        tfIndices.push_back(i);
    }

    glowl::Mesh::VertexDataList<float> histoData{
        {tfLocations, {4, {{1, GL_FLOAT, GL_FALSE, 0}}}},
        {tfData, {16, {{4, GL_FLOAT, GL_FALSE, 0}}}},
    };

    vaTransferFunc = std::make_unique<glowl::Mesh>(histoData, tfIndices, GL_UNSIGNED_INT, GL_LINE_STRIP, GL_STATIC_DRAW);

    glGenTextures(1, &tfTex);
    glBindTexture(GL_TEXTURE_1D, tfTex);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, histoNumBins, 0, GL_RGBA, GL_FLOAT, tfData.data());
    glBindTexture(GL_TEXTURE_1D, 0);
}

/**
 * @brief Update the transfer function, set all values of "channel" to "value".
 * @param channel  The channel to modify
 * @param value    The value to set the channel to
 */
void VolumeVis::updateTransferFunc(int channel, float value) {
    // --------------------------------------------------------------------------------
    //  TODO: Update the transfer function. Don't forget to update the texture and VA.
    // --------------------------------------------------------------------------------
    for (int i = 0; i < tfNumPoints; i++)
    {
        tfData[4*i+channel] = value;
    }
    initTransferFunc();
}

/**
 * @brief Update the transfer function, set the value at index "idx" of "channel" to "value".
 * @param idx      The index at which to modify the channel
 * @param channel  The channel to modify
 * @param value    The value to set the channel to at the given index
 */
void VolumeVis::updateTransferFunc(int idx, int channel, float value) {
    // --------------------------------------------------------------------------------
    //  TODO: Update the transfer function. Don't forget to update the texture and VA.
    // --------------------------------------------------------------------------------
    tfData[4*idx+channel] = value;
    initTransferFunc();
}

std::tuple<std::string, std::string> getNext(std::string s, std::string delimiter) {
    size_t i = s.find(delimiter);
    return std::make_tuple(s.substr(i+1),s.substr(0, i));
}

/**
 * @brief Load a transfer function from the given file.
 * @param filename The file to load the transfer function from
 */
void VolumeVis::loadTransferFunc(const std::string& filename) {
    auto path = getResourceDirPath("transfer") / filename;
    std::cout << "Load transfer function: " << path.string() << std::endl;
    // --------------------------------------------------------------------------------
    //  TODO: Load the transfer function from file "path".
    // --------------------------------------------------------------------------------
    std::string tf = getStringResource(path);
    std::string number;
    std::tie(tf, number) = getNext(tf, "\n");
    tfNumPoints = std::stoi(number);
    assert(tfNumPoints == histoNumBins && "tfNumPoints is not equal to histoNumBins");

    std::string rgba;
    for (int i = 0; i < tfNumPoints; i++)
    {
        std::tie(tf, rgba) = getNext(tf, "\n");
        // std::cout << rgba << std::endl;
        
        std::string data;

        //r
        std::tie(rgba, data) = getNext(rgba, " ");
        tfData.push_back(std::stof(data));
        //g
        std::tie(rgba, data) = getNext(rgba, " ");
        tfData.push_back(std::stof(data));
        //b
        std::tie(rgba, data) = getNext(rgba, " ");
        tfData.push_back(std::stof(data));
        //a
        tfData.push_back(std::stof(rgba));
    }
    
    // for (int i = 0; i < tfNumPoints; i++)
    // {
    //     int j = 4*i;
    //     std::cout << tfData[j] << " " << tfData[j+1] << " " << tfData[j+2] << " " << tfData[j+3] << std::endl;
    // }

}

/**
 * @brief Save the current transfer function to a file.
 * @param filename The filename to save the transfer function to.
 */
void VolumeVis::saveTransferFunc(const std::string& filename) {
    auto path = getResourceDirPath("transfer") / filename;
    std::cout << "Save transfer function: " << path.string() << std::endl;
    // --------------------------------------------------------------------------------
    //  TODO: Save transfer function to file "path".
    // --------------------------------------------------------------------------------
    std::ofstream f(path);
    f << tfNumPoints << std::endl;
    for (size_t i = 0; i < tfNumPoints; i++)
    {
        int j = 4*i;
        f << std::to_string(tfData[j]) << " " << std::to_string(tfData[j+1]) << " " << std::to_string(tfData[j+2]) << " " << std::to_string(tfData[j+3]) << std::endl;
    }
    
}
