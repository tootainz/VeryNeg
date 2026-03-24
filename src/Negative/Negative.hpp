#pragma once

#include <vector>
#include <fstream>
#include <tuple>

#include <nlohmann/json.hpp>

#include "ImageData.hpp"
#include "ImageArea.hpp"


/**

The Negative class

This is where all of the important stuff happens
Represents one scanned negative image. It stores and manages the negative image data.
Performs image editing operations to the image to convert it to positive and to edit it after the conversion.
Performs opening and exporting operations of the image
Also generates stuff for the GUI
Handles caching and reading NegativeData edit settings.

*/

class Negative {

private:
    // PRIVATE STATIC DATA MEMBERS
    // ------------------------------------------------------------------------------------------------------------------------------------

    static int nextId;
    static const int PREVIEW_SIZE = 800;
    static const int THUMBNAIL_SIZE = 500;
    static const int EYEDROPPER_SIZE = 5;
    static const int SHARPNESS_PREVIEW_SIZE = 100;


    // PRIVATE DATA MEMBERS
    // ------------------------------------------------------------------------------------------------------------------------------------

    // General data
    int id;
    std::string name;
    std::filesystem::path path;

    // Original specs and data of the image
    // All pixel data is stored in a one dimensional array, where pixels are stored sequentially with their channels as well in the order of RGB.
    // For example: [1R, 1G, 1B, 2R, 2G, 2B, 3R, 3G, 3B, ...]
    std::vector<float> originalPixels;
    int numberOfChannels;
    int width;
    int height;
    
    // Working image is a smaller version of the original in order to speed up the live editing process
    std::vector<float> workingPixels;
    std::vector<float> convertedPixels;     // Pixels after the negative conversion
    std::vector<float> editedPixels;        // Pixels after applying all the post-covnert edits
    int workingScale;                       // How much the working image is scaled down from the original, calculated automatically to fit UI
    int workingWidth;
    int workingHeight;

    // Thumbnail for UI
    std::vector<uint8_t> thumbnailPixels;

    // Sharpness Preview
    std::vector<uint8_t> sharpnessPreviewPixels;

    // Edit settings that are saved for future sessions
    nlohmann::json negativeData;


private:
    // PRIVATE METHODS
    // ------------------------------------------------------------------------------------------------------------------------------------

    // PREVIEW CACHING
    bool writeConversionCache();
    bool readConversionCache();

    // NEGATIVE DATA IO
    void readNegativeData();
    void writeNegativeData();

    // HELPERS
    std::tuple<float,float,float> samplePixels(int workingX, int workingY);

    // INITIALIZER
    bool initializeNegative(std::filesystem::path imagePath);


public:
    // PUBLIC METHODS
    // ------------------------------------------------------------------------------------------------------------------------------------

    // CONSTRUCTORS
    Negative(std::filesystem::path imagePath);

    // GETTERS FOR THE UI
    ImageData getPreview();
    ImageData getThumbnail();
    ImageData getSharpnessPreview();
    int getId();

    // EXPORTING
    bool exportPositive(std::filesystem::path imagePath);

    // SETTING EDIT SETTINGS

    // PRE-CONVERT
    float setScanGamma(float value);
    void setScanArea(ImageArea area);
    void setBorder(float r, float g, float b);
    void setBorderByCoords(int x, int y);
    void setDensest(float r, float g, float b);
    void setDensestByCoords(int x, int y);
    void convert();
    void resetConversion();

    // POST-CONVERT
    // Intensity
    float setDensity(float value);
    float setContrast(float value);
    float setWhites(float value);
    float setHighlights(float value);
    float setShadows(float value);
    float setBlacks(float value);

    // White balance
    void setNeutral(float r, float g, float b);
    void setNeutralByCoords(int x, int y);
    float setRBalance(float value);
    float setGBalance(float value);
    float setBBalance(float value);

    // GETTING EDIT SETTINGS FROM NEGATIVEDATA

    // PRE-CONVERT
    float getScanGamma();
    std::tuple<float, float, float> getBorder();
    std::tuple<float, float, float> getDensest();
    ImageArea getScanArea();

    // POST-CONVERT
    // Intensity
    float getDensity();
    float getContrast();
    float getWhites();
    float getHighlights();
    float getShadows();
    float getBlacks();

    // White balance
    std::tuple<float, float, float> getNeutral();
    float getRBalance();
    float getGBalance();
    float getBBalance();

    // Rendering methods
    void renderThumbnail();
    void renderEdits();
    void renderWorking();
    void renderFinal();
};