#pragma once

#include <print>
#include <algorithm>
#include <cmath> 

#include "EditChannel.hpp"
#include "iterateImage.hpp"

inline const float HIGHLIGHT_PROTECTION_AMOUNT = 1.2f;

inline float curveExposureFunction(float input, float value) {

    if (value >= 1.0f) {
        return (1-(-value))*input + (-value)*std::pow(input, HIGHLIGHT_PROTECTION_AMOUNT);
    }
    else {
        float firstMin = std::min(1.0f/std::pow(value, 10.0f), 0.5f + 1.0f/std::pow(value, 8.0f));
        float secondMin = std::min(2.0f + 1.0f/ std::pow(value, 4.0f), 7.0f);
        float gamma = std::min(firstMin, secondMin);

        return 0.5*value*input + 0.5*value*std::pow(input, gamma);
    }
}

inline void curveExposure(std::vector<float>& image, float value, EditChannel channel) {
    std::println("exposure value is: {}", value);

    auto applyCurveExposure = [&](float& red, float& green, float& blue) {
        if (channel == EditChannel::RGB) {
            red = curveExposureFunction(red, value);
            green = curveExposureFunction(green, value);
            blue = curveExposureFunction(blue, value);
        } else if (channel == EditChannel::R) {
            red = curveExposureFunction(red, value);
        } else if (channel == EditChannel::G) {
            green = curveExposureFunction(green, value);
        } else {
            blue = curveExposureFunction(blue, value);
        }
    };
    iterateImageMutableMultiThread(image, applyCurveExposure);
    return;
};