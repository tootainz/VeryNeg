#pragma once

#include <print>

#include "EditChannel.hpp"
#include "iterateImage.hpp"

inline float multiplyFunction(float input, float value) {
    return input * value;
}

inline void multiply(std::vector<float>& image, float multiplier, EditChannel channel) {

    auto applyMultiply = [&](float& red, float& green, float& blue) {
        if (channel == EditChannel::RGB) {
            red = multiplyFunction(red, multiplier);
            green = multiplyFunction(green, multiplier);
            blue = multiplyFunction(blue, multiplier);
        } else if (channel == EditChannel::R) {
            red = multiplyFunction(red, multiplier);
        } else if (channel == EditChannel::G) {
            green =multiplyFunction(green, multiplier);
        } else {
            blue = multiplyFunction(blue, multiplier);
        }
    };
    iterateImageMutable(image, applyMultiply);
    return;
};