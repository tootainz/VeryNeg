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

    void changeCurrentNegativeById(int id);
    void previousNegative();
    void nextNegative();

    Negative& addNegative(std::filesystem::path imagePath);
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