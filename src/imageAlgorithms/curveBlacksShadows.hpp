#pragma once

#include <print>
#include <algorithm>
#include <cmath> 

#include "EditChannel.hpp"
#include "iterateImage.hpp"


int const BLACKS_SHARPNESS = 5;
int const SHADOWS_SHARPNESS = 1;
float const INCREASE_DAMPENING = 0.1f;
float const SHARPNESS_MULTIPLIER = 10.0f;
float const DECREASE_SLOPE_DAMPENING = 0.1f;

inline float darksFunction(float input, float value, int sharpness) {

    // Make sure sharpness is odd
    int correctedSharpness = sharpness;
    if (sharpness % 2 == 1) {
        correctedSharpness += 1;
    }

    if (value >= 0.0f) {
        // 1 + (1-d)(x-1) + d(x-1)^sharpness
        // sharpness needs multiplying as well
        // d = adjustment amount * dampening

        float amount = value*INCREASE_DAMPENING;
        float multipliedSharpness = correctedSharpness * SHARPNESS_MULTIPLIER; 

        return 1 + (1-amount)*(input-1) + amount*std::pow(input - 1, correctedSharpness);
    }
    else {
        // cx + (1-c)(bx)/((b-1)+|x|^d)
        // c = linear section slope * dampening
        // b = sharpness * multiplier
        // d = adjustment amount

        float amount = value;
        float multipliedSharpness = correctedSharpness * SHARPNESS_MULTIPLIER; 
        float slope = 1/sharpness*DECREASE_SLOPE_DAMPENING;
        float absoluteInput = std::abs(input);

        return slope*input + (1-slope)*(multipliedSharpness*input) / ((multipliedSharpness-1) + std::pow(absoluteInput, amount));
    }
}

inline float blacksFunction(float input, float value) {
    return darksFunction(input, value, BLACKS_SHARPNESS);
}

inline float shadowsFunction(float input, float value) {
    return darksFunction(input, value, SHADOWS_SHARPNESS);
}

inline void blacks(std::vector<float>& image, float value) {
    std::println("exposure value is: {}", value);

    auto applyBlacks = [&](float& red, float& green, float& blue) {
        red = blacksFunction(red, value);
        green = blacksFunction(green, value);
        blue = blacksFunction(blue, value);
    };

    iterateImageMutableMultiThread(image, applyBlacks);
    return;
};

inline void shadows(std::vector<float>& image, float value) {
    std::println("exposure value is: {}", value);

    auto applyShadows = [&](float& red, float& green, float& blue) {
        red = shadowsFunction(red, value);
        green = shadowsFunction(green, value);
        blue = shadowsFunction(blue, value);
    };

    iterateImageMutableMultiThread(image, applyShadows);
    return;
};