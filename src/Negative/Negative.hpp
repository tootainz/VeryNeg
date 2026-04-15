#pragma once

#include <vector>
#include <fstream>
#include <tuple>
#include <optional>

#include <nlohmann/json.hpp>
#include <lcms2.h>

#include "ImageData.hpp"
#include "ImageArea.hpp"
#include "../ColorProfiler/ColorProfiler.hpp"


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
    static const int PREVIEW_SIZE = 1000;
    static const int THUMBNAIL_SIZE = 500;
    static const int EYEDROPPER_SIZE = 5;
    static const int SHARPNESS_PREVIEW_SIZE = 200;
    static const std::string NEGATIVEDATA_VERSION;
    static const std::string PRESETDATA_VERSION;
    static const float DRAGGING_SCALE;


    // PRIVATE DATA MEMBERS
    // ------------------------------------------------------------------------------------------------------------------------------------

    // General data
    int id;
    std::string name;
    std::filesystem::path path;
    bool successfullyCreated;

    // Original specs and data of the image
    // All pixel data is stored in a one dimensional array, where pixels are stored sequentially with their channels as well in the order of RGB.
    // For example: [1R, 1G, 1B, 2R, 2G, 2B, 3R, 3G, 3B, ...]
    std::vector<float> originalPixels;
    int numberOfChannels;
    int width;
    int height;
    std::optional<std::vector<uint8_t>> iccProfile = std::nullopt;
    
    // Working image is a smaller version of the original in order to speed up the live editing process
    std::vector<float> workingPixels;
    std::vector<float> convertedPixels;         // Pixels after the negative conversion
    std::vector<float> convertedDraggingPixels; // Pixels after the conversion, but used when dragging a slider to improve preview performance
    std::vector<float> editedPixels;            // Pixels after applying all the post-covnert edits
    std::vector<float> editedDraggingPixels;    // Pixels after the edits, but used when dragging a slider to improve preview performance
    float workingScale;                           // How much the working image is scaled down from the original, calculated automatically to fit UI
    int workingWidth;
    int workingHeight;

    // Thumbnail for UI
    std::vector<uint8_t> thumbnailPixels;

    // Sharpness Preview
    std::vector<float> sharpnessPreviewOriginalPixels;
    std::vector<float> sharpnessPreviewConvertedPixels;
    std::vector<float> sharpnessPreviewEditedPixels;

    // Edit settings that are saved for future sessions
    nlohmann::json negativeData;

    // Reference to the color profiler for transforms
    ColorProfiler* profiler;

private:
    // PRIVATE METHODS
    // ------------------------------------------------------------------------------------------------------------------------------------

    // PREVIEW CACHING
    bool writeConversionCache();
    bool readConversionCache();

    // NEGATIVE DATA IO
    void readNegativeData();
    void writeNegativeData();

    // PRESET DATA
    std::unique_ptr<nlohmann::json> readPresetdata(std::filesystem::path path);

    // HELPERS
    std::tuple<float,float,float> samplePixels(int workingX, int workingY);

    // Internal rendering methods
    std::unique_ptr<std::vector<float>> renderFinal();
    void renderEdits(std::vector<float>& pixels, int channels);
    void renderConversion(std::vector<float>& pixels, int width, int height, int channels, float scale);

    // Sharpness preview
    void initializeSharpnessPreview();

    // INITIALIZER
    bool initializeNegative(std::filesystem::path imagePath);


public:
    // PUBLIC METHODS
    // ------------------------------------------------------------------------------------------------------------------------------------

    // CONSTRUCTORS
    Negative(std::filesystem::path imagePath, ColorProfiler* profiler);
    Negative(std::filesystem::path imagePath, ColorProfiler* profiler, int id); // IMPORTANT! Call this only when undoing a removeNegativeById
    bool wasCreated(); // Tells whether intialization was succesful

    // GETTERS FOR THE UI
    ImageData getPreview(bool dragging);
    ImageData getThumbnail();
    ImageData getSharpnessPreview();
    int getId();
    int getWorkingWidth();
    int getWorkingHeight();
    float getWorkingScale();
    std::filesystem::path getPath();

    // EXPORTING
    bool exportPositive(std::filesystem::path imagePath, std::string imageFormat, std::string iccProfile);

    // SETTING EDIT SETTINGS

    // PRE-CONVERT
    float setScanGamma(float value);
    void setHasScanArea(bool has);
    void setScanArea(ImageArea area, float scale);
    void setHasBorder(bool has);
    void setBorder(float r, float g, float b);
    void setBorderByCoords(int x, int y);
    void setHasDensest(bool has);
    void setDensest(float r, float g, float b);
    void setDensestByCoords(int x, int y);
    void convert();
    void resetConversion();

    // POST-CONVERT
    void applyPreset(std::filesystem::path presetPath);

    // Intensity
    float setDensity(float value);
    float setContrast(float value);
    float setWhites(float value);
    float setHighlights(float value);
    float setShadows(float value);
    float setBlacks(float value);

    // White balance
    void setAutoWB(bool has);
    void setHasNeutral(bool has);
    void setNeutral(float r, float g, float b);
    void setNeutralByCoords(int x, int y);
    float setRBalance(float value);
    float setGBalance(float value);
    float setBBalance(float value);
    float setSaturation(float value);

    // Sharpening
    float setSharpeningAmount(float value);
    float setSharpeningDiameter(float value);

    // GETTING EDIT SETTINGS FROM NEGATIVEDATA

    nlohmann::json getNegativeData();

    // PRE-CONVERT
    float getScanGamma();
    bool getHasBorder();
    std::tuple<float, float, float> getBorder();
    bool getHasDensest();
    std::tuple<float, float, float> getDensest();
    bool getHasScanArea();
    ImageArea getScanArea(float scale);
    bool getIsConverted();

    // POST-CONVERT
    // Intensity
    float getDensity();
    float getContrast();
    float getWhites();
    float getHighlights();
    float getShadows();
    float getBlacks();

    // White balance
    bool getAutoWB();
    bool getHasNeutral();
    std::tuple<float, float, float> getNeutral();
    float getRBalance();
    float getGBalance();
    float getBBalance();
    float getSaturation();

    // Sharpening
    float getSharpeningAmount();
    float getSharpeningDiameter();

    // Public rendering methods
    void renderWorkingConversion();
    void renderWorkingEdits();
    void renderDraggingConversion();
    void renderDraggingEdits();
    void renderSharpnessPreviewConversion();
    void renderSharpnessPreviewEdits();
    void renderThumbnail();
};