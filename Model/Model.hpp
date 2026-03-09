#pragma once

#include "../Negative/Negative.hpp"

class Model {

    private:

        Negative negative;

    public:

    Model() :
        negative("input.tif")
    {}

    void render() {
        negative.render();
    }

    void renderEdits() {
        negative.renderEdits();
    }

    void initializeNegative(std::string imagePath) {
        this->negative.initializeNegative(imagePath);
    }

    void savePositive(std::string imagePath) {
        this->negative.savePositive(imagePath);
    }

    void setExposure(float value) {
        this->negative.setExposure(value);
    };

    void setBSlope(float value) {
        this->negative.setBSlope(value);
    }

    void setGSlope(float value) {
        this->negative.setGSlope(value);
    }

    void setRBalance(float value) {
        this->negative.setRBalance(value);
    }

    void setGBalance(float value) {
        this->negative.setGBalance(value);
    }

    void setBBalance(float value) {
        this->negative.setBBalance(value);
    }


    ImageData getPreview() {
        return negative.getPreview();
    }
};