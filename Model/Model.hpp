#pragma once

#include <vector>
#include <filesystem>

#include "../Negative/Negative.hpp"


class Model {

private:

    // PRIVATE DATA MEMBERS

    /// A Vector that stores all the negatives loaded to the application
    std::vector<Negative> negatives;
    
    /// Stores the index of the negative that is currently selected and should be displayed
    int currentNegativeIndex;


public:

    // CONSTRUCTOR
    Model();


    // RENDER FUNCTIONS FOR IMAGE DATA
    void renderWorking();
    void renderEdits();
    void renderFinal();


    // GUI INTERACTIONS

    // NEGATIVE NAVIGATION
    void changeCurrentNegativeById(int id);
    void previousNegative();
    void nextNegative();
    Negative& addNegative(std::filesystem::path imagePath);
    void removeNegative(int i);

    // EXPORT
    void exportPositive(std::string imagePath);

    // PRE-CONVERT
    float setScanGamma(float value);
    void setBorder();
    void setDensest();
    void setScanArea();
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
    void autoWhiteBalance();
    void chooseNeutralBalance();
    float setRBalance(float value);
    float setGBalance(float value);
    float setBBalance(float value);


    // GETTERS

    // Settings values
    float getScanGamma();

    float getDensity();
    float getContrast();
    float getWhites();
    float getHighlights();
    float getShadows();
    float getBlacks();

    float getRBalance();
    float getGBalance();
    float getBBalance();
    
    // Image data
    // Histogram getHistogram(); // TODO if have interest and time
    ImageData getPreview();
    ImageData getThumbnail(int id);
    int getCurrentNegativeId();
};