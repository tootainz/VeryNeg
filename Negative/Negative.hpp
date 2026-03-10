#pragma once

#include <vector>

struct ImageData {
    std::vector<uint8_t> data;
    int width;
    int height;
};

class Negative {

    private:
    std::vector<float> pixels;
    std::vector<float> convertedPixels;
    std::vector<float> editedPixels;
    int width;
    int height;
    int numberOfChannels;

    // Edit stats, should these be their own class??
    float exposure = 0.0f;
    float rBalance = 1.0f;
    float gBalance = 1.0f;
    float bBalance = 1.0f;
    float blackPoint = 0.0f;
    float whitePoint = 0.0f;
    float highlights = 0.0f;
    float shadows = 0.0f;
    float sharpening = 0.0f;

    float rBlack = 0.0f;
    float gBlack = 0.0f;
    float bBlack = 0.0f;

    float rMiddle = 0.0f;
    float gMiddle = 0.0f;
    float bMiddle = 0.0f;

    float rWhite = 0.0f;
    float gWhite = 0.0f;
    float bWhite = 0.0f;

    public:
    
    Negative(std::string imagePath);
    Negative();

    bool initializeNegative(std::string imagePath);
    bool savePositive(std::string imagePath);
    
    // Returns a preview to show in the GUI in the form of ImageData. This will be shown with SFML, The colors are assumed to be sRGB in the preview
    ImageData getPreview();

    void setExposure(float value);

    void setRBalance(float value);
    void setGBalance(float value);
    void setBBalance(float value);

    void renderEdits();

    // Renders the initial conversion with the given pipeline
    void render();
};