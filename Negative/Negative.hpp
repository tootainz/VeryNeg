#pragma once

#include <vector>
#include <nlohmann/json.hpp>
#include <fstream>

struct ImageData {
    std::vector<uint8_t> data;
    int width;
    int height;
};

class Negative {

    private:

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

    std::vector<uint8_t> thumbnailPixels;

    // Edit settings
    nlohmann::json negativeData;

    public:
    
    Negative(std::string imagePath);
    Negative();

    bool initializeNegative(std::string imagePath);
    bool savePositive(std::string imagePath);
    
    // Returns a preview to show in the GUI in the form of ImageData. This will be shown with SFML, The colors are assumed to be sRGB in the preview
    ImageData getPreview();
    ImageData getSharpnessPreview();

    // Reading and writing the edit data
    void readNegativeData();
    void writeNegativeData();

    void setExposure(float value);
    void setRBalance(float value);
    void setGBalance(float value);
    void setBBalance(float value);

    bool cacheConversion();
    bool readCacheConversion();

    void renderThumbnail();
    void renderEdits();
    void renderWorking();
    void renderFinal();
};