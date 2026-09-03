#pragma once

#include <algorithm>
#include <cmath> 

#include "EditChannel.hpp"
#include "iterateImage.hpp"
#include "../debug_print.hpp"

inline const float HIGHLIGHT_PROTECTION_AMOUNT = 1.14f;
inline const float SHADOW_DAMPENER_FIXER = 1.075f;

inline float curveExposureFunction(float input, float value) {

    if (value >= 0.0f) {
        float result = (1.0f+value)*input - value*std::pow(input, HIGHLIGHT_PROTECTION_AMOUNT);
        return std::clamp(result, 0.0f, 1.0f);
    }
    else {
        float correctedValue = std::pow(SHADOW_DAMPENER_FIXER, value);
        float firstMin = std::min(1.0f/(correctedValue*correctedValue*correctedValue), 0.5f + 1.0f/(correctedValue*correctedValue));
        float secondMin = std::min(2.0f + 1.0f/correctedValue, 7.0f);
        float gamma = std::min(firstMin, secondMin);

        float result = 0.5f*correctedValue*input + 0.5f*(1.0f - ((1.0f-correctedValue)*(1.0f-correctedValue)*(1.0f-correctedValue)))*std::pow(input, gamma);
        return std::clamp(result, 0.0f, 1.0f);
    }
}

inline void curveExposure(std::vector<float>& image, float value, EditChannel channel) {
    DEBUG_PRINT("exposure value is: {}", value);

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