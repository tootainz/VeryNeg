#pragma once

#include <print>

#include "gamma.hpp"
#include "EditChannel.hpp"

inline float myExposureFunction(float input, float value) {
    return input * value;
}

inline void myExposure(std::vector<float>& image, float value, EditChannel channel) {

    float finalValue;
    if (value == 0) {
        return;
    }
    else if (value > 0) {
        finalValue = value;
    } else { // since 0 is taken care of we should never divide by 0
        finalValue = 1.0f/-value;
    }
    std::println("Exposure value is: {}", finalValue);

    auto applyMyExposure = [&](float& red, float& green, float& blue) {
        if (channel == EditChannel::RGB) {
            red = myExposureFunction(red, finalValue);
            green = myExposureFunction(green, finalValue);
            blue = myExposureFunction(blue, finalValue);
        } else if (channel == EditChannel::R) {
            red = myExposureFunction(red, finalValue);
        } else if (channel == EditChannel::G) {
            green = myExposureFunction(green, finalValue);
        } else {
            blue = myExposureFunction(blue, finalValue);
        }
    };
    iterateImageMutable(image, applyMyExposure);
    return;
};