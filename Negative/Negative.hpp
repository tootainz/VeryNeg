#pragma once

#include <vector>

struct ImageData {
    std::vector<uint8_t> data;
    int width;
    int height;
};

class Negative {

    public:
    std::vector<float> pixels;
    std::vector<float> convertedPixels;
    std::vector<float> editedPixels;
    int width;
    int height;
    int numberOfChannels;

    float exposure;
    float bSlope = 1;
    float gSlope = 1;
    
    Negative(std::string imagePath);

    Negative();

    bool initializeNegative(std::string imagePath);

    bool savePositive(std::string imagePath);
    
    // Returns a preview to show in the GUI in the form of ImageData. This will be shown with SFML, The colors are assumed to be sRGB in the preview
    ImageData getPreview();

    void setExposure(float value);

    void setBSlope(float value);
    void setGSlope(float value);

    void renderEdits();

    // Renders the initial conversion with the given pipeline
    void render();
};