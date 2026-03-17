#pragma once

#include <vector>
#include <cmath>

#include "iterateImage.hpp"
#include "EditChannel.hpp"


inline float gammaFunction(float inputValue, float gamma) {
    return std::pow(inputValue, gamma);
}

inline void gamma(std::vector<float>& image, float gamma, EditChannel channel) {
    auto applyGamma = [&](float& red, float& green, float& blue) {
        if (channel == EditChannel::RGB) {
            red = gammaFunction(red, gamma);
            green = gammaFunction(green, gamma);
            blue = gammaFunction(blue, gamma);
        } else if (channel == EditChannel::R) {
            red = gammaFunction(red, gamma);
        } else if (channel == EditChannel::G) {
            green = gammaFunction(green, gamma);
        } else {
            blue = gammaFunction(blue, gamma);
        }
    };
    iterateImageMutableMultiThread(image, applyGamma);
    return;
}