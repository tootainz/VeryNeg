#pragma once

#include <print>
#include <algorithm>
#include <cmath> 

#include "EditChannel.hpp"
#include "iterateImage.hpp"


int const WHITES_SHARPNESS = 5;
int const HIGHLIGHTS_SHARPNESS = 1;
float const W_INCREASE_DAMPENING = 0.1f;
float const W_SHARPNESS_MULTIPLIER = 10.0f;
float const W_DECREASE_SLOPE_DAMPENING = 0.1f;

inline float lightsFunction(float input, float value, int sharpness) {

    // Make sure sharpness is odd
    int correctedSharpness = sharpness;
    if (sharpness % 2 == 0) {
        correctedSharpness += 1;
    }

    if (value <= 0.0f) {
        // (1-d)x + dx^sharpness
        // sharpness needs multiplying as well
        // d = adjustment amount * dampening

        float amount = -value*W_INCREASE_DAMPENING;
        float multipliedSharpness = correctedSharpness * W_SHARPNESS_MULTIPLIER; 

        return (1-amount)*input + amount*std::pow(input, correctedSharpness);
    }
    else {
        // 1 + c(x-1) + (1-c)(b(x-1))/((b-1)+|(x-1)|^d)
        // c = linear section slope * dampening
        // b = sharpness * multiplier
        // d = adjustment amount

        float amount = value;
        float multipliedSharpness = correctedSharpness * W_SHARPNESS_MULTIPLIER; 
        float slope = 1/sharpness*W_DECREASE_SLOPE_DAMPENING;
        float absoluteInput = std::abs(input-1);

        return 1 + slope*(input-1) + (1-slope)*(multipliedSharpness*(input-1)) / ((multipliedSharpness-1) + std::pow(absoluteInput, -amount));
    }
}

inline float whitesFunction(float input, float value) {
    return lightsFunction(input, value, WHITES_SHARPNESS);
}

inline float highlightsFunction(float input, float value) {
    return lightsFunction(input, value, HIGHLIGHTS_SHARPNESS);
}

inline void whites(std::vector<float>& image, float value) {
    std::println("exposure value is: {}", value);

    auto applyWhites = [&](float& red, float& green, float& blue) {
        red = whitesFunction(red, value);
        green = whitesFunction(green, value);
        blue = whitesFunction(blue, value);
    };

    iterateImageMutableMultiThread(image, applyWhites);
    return;
};

inline void highlights(std::vector<float>& image, float value) {
    std::println("exposure value is: {}", value);

    auto applyHighlights = [&](float& red, float& green, float& blue) {
        red = highlightsFunction(red, value);
        green = highlightsFunction(green, value);
        blue = highlightsFunction(blue, value);
    };

    iterateImageMutableMultiThread(image, applyHighlights);
    return;
};