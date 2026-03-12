#pragma once

#include <vector>
#include <filesystem>

#include "../Negative/Negative.hpp"


class Model {

    public:

        std::vector<Negative> negatives;
        int currentNegativeIndex;

    public:

    Model();

    void renderWorking();
    void renderEdits();

    void changeCurrentNegativeByIndex(int i);
    void previousNegative();
    void nextNegative();

    ImageData addNegative(std::filesystem::path imagePath);
    void removeNegative(int i);

    void exportPositive(std::string imagePath);

    void setExposure(float value);
    void setRBalance(float value);
    void setGBalance(float value);
    void setBBalance(float value);

    float getExposure();
    float getRBalance();
    float getGBalance();
    float getBBalance();
    
    ImageData getPreview();
};