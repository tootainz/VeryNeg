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

    ImageData getPreview() {
        return negative.getPreview();
    }
};