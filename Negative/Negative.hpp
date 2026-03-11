#pragma once

#include <vector>
#include <nlohmann/json.hpp>
#include <fstream>
#include <tuple>
#include "ImageData.hpp"
#include "ImageArea.hpp"

class Negative {

    private:

    static int nextId;

    int id;
    std::string name;
    std::filesystem::path path;

    // Original specs and data of the image
    std::vector<float> originalPixels;
    int numberOfChannels;
    int width;
    int height;
    
    // Working image is a smaller version of the original in order to speed up the live editing process
    std::vector<float> workingPixels;
    std::vector<float> convertedPixels;
    std::vector<float> editedPixels;
    int workingScale;
    int workingWidth;
    int workingHeight;
    ImageArea scanArea;

    std::vector<uint8_t> thumbnailPixels;

    // Edit settings
    nlohmann::json negativeData;

    public:
    
    Negative(std::filesystem::path imagePath);
    Negative();

    bool initializeNegative(std::filesystem::path imagePath);
    
    bool exportPositive(std::filesystem::path imagePath);
    
    // Returns a preview to show in the GUI in the form of ImageData. This will be shown with SFML, The colors are assumed to be sRGB in the preview
    ImageData getPreview();
    ImageData getSharpnessPreview();
    ImageData getThumbnail();

    // Reading and writing the edit data
    void readNegativeData();
    void writeNegativeData();

    void setExposure(float value);
    void setRBalance(float value);
    void setGBalance(float value);
    void setBBalance(float value);

    bool writeConversionCache();
    bool readConversionCache();

    void renderThumbnail();
    void renderEdits();
    void renderWorking();
    void renderFinal();
};