#pragma once

#include <vector>
#include <filesystem>

#include "../Negative/Negative.hpp"

class Model {

    public:

        std::vector<Negative> negatives;
        int currentNegativeIndex;

        // crop region
        // white point region
        // black point region
        // middle grey region
        // sharpness region

    public:

    Model() {}

    void renderWorking() {
        this->negatives[this->currentNegativeIndex].renderWorking();
    }
    void renderEdits() {
        this->negatives[this->currentNegativeIndex].renderEdits();
    }

    void changeCurrentNegativeByIndex(int i) {
        this->currentNegativeIndex = i;
        std::println("current negative is: {}", this->currentNegativeIndex);
    }

    void previousNegative() {
        this->currentNegativeIndex = std::max(0, this->currentNegativeIndex - 1);
        std::println("current negative is: {}", this->currentNegativeIndex);
    }

    void nextNegative() {
        int size = this->negatives.size();
        this->currentNegativeIndex = std::min(size - 1, this->currentNegativeIndex + 1);
        std::println("current negative is: {}", this->currentNegativeIndex);
    }

    ImageData addNegative(std::filesystem::path imagePath) {
        this->negatives.push_back(Negative(imagePath));
        if (negatives.size() == 1) currentNegativeIndex = 0;
        else currentNegativeIndex += 1;
        return this->negatives[this->currentNegativeIndex].getThumbnail();
    }

    void removeNegative(int i);

    void savePositive(std::string imagePath) {};

    void setExposure(float value) {
        this->negatives[this->currentNegativeIndex].setExposure(value);
    };
    void setRBalance(float value) {
        this->negatives[this->currentNegativeIndex].setRBalance(value);
    };
    void setGBalance(float value) {
        this->negatives[this->currentNegativeIndex].setGBalance(value);
    };
    void setBBalance(float value) {
        this->negatives[this->currentNegativeIndex].setBBalance(value);
    };

    float getExposure();
    float getRBalance();
    float getGBalance();
    float getBBalance();
    
    ImageData getPreview() {
        return this->negatives[this->currentNegativeIndex].getPreview();
    }
};