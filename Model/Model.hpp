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

    void initializeNegative(std::string imagePath) {
        this->negative.initializeNegative(imagePath);
    }

    ImageData getPreview() {
        return negative.getPreview();
    }
};