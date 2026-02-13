#pragma once

#include <vector>
#include <cmath>

#include "iterateImage.hpp"

enum class GammaChannel {
    RGB,
    R,
    G,
    B
};

float gammaFunction(float inputValue, float gamma) {
    return std::pow(inputValue, gamma);
}

void gamma(std::vector<float>& image, float gamma, GammaChannel channel) {
    auto applyGamma = [&](float& red, float& green, float& blue) {
        if (channel == GammaChannel::RGB) {
            red = gammaFunction(red, gamma);
            green = gammaFunction(green, gamma);
            blue = gammaFunction(blue, gamma);
        } else if (channel == GammaChannel::R) {
            red = gammaFunction(red, gamma);
        } else if (channel == GammaChannel::G) {
            green = gammaFunction(green, gamma);
        } else {
            blue = gammaFunction(blue, gamma);
        }
    };
    iterateImageMutable(image, applyGamma);
    return;
}