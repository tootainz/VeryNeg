#pragma once

#include <print>

#include "EditChannel.hpp"
#include "iterateImage.hpp"


inline float contrastFunction(float input, float value) {;
    return std::pow(input, value)/(std::pow(input, value) + std::pow(1-input, value));
}

inline void contrast(std::vector<float>& image, float multiplier, EditChannel channel) {

    auto applyContrast = [&](float& red, float& green, float& blue) {
        if (channel == EditChannel::RGB) {
            red = contrastFunction(red, multiplier);
            green = contrastFunction(green, multiplier);
            blue = contrastFunction(blue, multiplier);
        } else if (channel == EditChannel::R) {
            red = contrastFunction(red, multiplier);
        } else if (channel == EditChannel::G) {
            green =contrastFunction(green, multiplier);
        } else {
            blue = contrastFunction(blue, multiplier);
        }
    };
    iterateImageMutableMultiThread(image, applyContrast);
    return;
};