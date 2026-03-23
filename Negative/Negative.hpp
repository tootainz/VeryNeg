#pragma once

#include <vector>
#include <fstream>
#include <tuple>

#include <nlohmann/json.hpp>

#include "ImageData.hpp"
#include "ImageArea.hpp"


// The Negative class is where all of the important stuff happens
// Represents one scanned negative image. It stores and manages the negative image data.
// Performs image editing operations to the image to convert it to positive and to edit it after the conversion.
// Performs opening and exporting operations of the image
// Also generates stuff for the GUI
// Handles caching and reading NegativeData edit settings.

class Negative {

private:

    // STATIC DATA MEMBERS
    static int nextId;
    static const int PREVIEW_SIZE = 800;
    static const int THUMBNAIL_SIZE = 500;
    static const int SAMPLE_SIZE = 5;

    // PRIVATE DATA MEMBERS

    // General data
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

    // Thumbnail
    std::vector<uint8_t> thumbnailPixels;

    // Edit settings
    nlohmann::json negativeData;


public:

    // Constructors and initializers
    Negative(std::filesystem::path imagePath);
    bool initializeNegative(std::filesystem::path imagePath);
    
    // Getters for image data
    ImageData getPreview();
    ImageData getSharpnessPreview();
    ImageData getThumbnail();
    int getId();

    // Exporting
    bool exportPositive(std::filesystem::path imagePath);

    // Reading and writing the edit data
    void readNegativeData();
    void writeNegativeData();

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

    // GETTING EDIT SETTINGS
    float getScanGamma();
    std::tuple<float, float, float> getBorder();
    std::tuple<float, float, float> getDensest();
    ImageArea getScanArea();

    float getDensity();
    float getContrast();
    float getWhites();
    float getHighlights();
    float getShadows();
    float getBlacks();

    std::tuple<float, float, float> getNeutral();
    float getRBalance();
    float getGBalance();
    float getBBalance();

    // Caching methods
    bool writeConversionCache();
    bool readConversionCache();

    // Helper for sampling an average from a specific area of the image
    std::tuple<float,float,float> samplePixels(int workingX, int workingY);

    // Rendering methods
    void renderThumbnail();
    void renderEdits();
    void renderWorking();
    void resetWorking();
    void renderFinal();
};