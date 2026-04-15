#pragma once

#include <print>
#include <algorithm>
#include <cmath> 

#include "EditChannel.hpp"
#include "iterateImage.hpp"

const float GENERAL_DAMPENING = 1.0f;

// Blacks
const float BLACKS_INCREASE_DAMPENING = 0.1f;
const int BLACKS_INCREASE_SHARPNESS = 5.0f;
const int BLACKS_DECREASE_SHARPNESS = 40.0f;

inline float blacksFunction(float input, float value) {

    if (value >= 0.0f) {
        float amount = value*BLACKS_INCREASE_DAMPENING;

        float result = (1.0f-amount)*input + amount*std::pow(input, 1.0f/BLACKS_INCREASE_SHARPNESS);
        return std::clamp(result, 0.0f, 1.0f);
    }
    else {
        float amount = value;
        float absoluteInput = std::abs(input);

        float result = (BLACKS_DECREASE_SHARPNESS*input) / ((BLACKS_DECREASE_SHARPNESS-1.0f) + std::pow(absoluteInput, amount));
        return std::clamp(result, 0.0f, 1.0f);
    }
}


inline void blacks(std::vector<float>& image, float value) {

    auto applyBlacks = [&](float& red, float& green, float& blue) {
        red = blacksFunction(red, value);
        green = blacksFunction(green, value);
        blue = blacksFunction(blue, value);
    };

    iterateImageMutableMultiThread(image, applyBlacks);
    return;
};

// Shadows
const float SHADOWS_INCREASE_DAMPENING = 0.1f;
const int SHADOWS_INCREASE_SHARPNESS = 15.0f;
const int SHADOWS_DECREASE_SHARPNESS = 9.0f;
const int SHADOWS_DECREASE_SLOPE = 0.08f;

inline float shadowsFunction(float input, float value) {

    if (value >= 0.0f) {
        float amount = value*SHADOWS_INCREASE_DAMPENING;

        float result = 1.0f + (1.0f-amount)*(input-1.0f) + amount*std::pow(input-1.0f, SHADOWS_INCREASE_SHARPNESS);
        return std::clamp(result, 0.0f, 1.0f);
    }
    else {
        float amount = value;
        float absoluteInput = std::abs(input);

        float result = SHADOWS_DECREASE_SLOPE*input + (1.0f-SHADOWS_DECREASE_SLOPE)*(SHADOWS_DECREASE_SHARPNESS*input) / ((SHADOWS_DECREASE_SHARPNESS-1.0f) + std::pow(absoluteInput, amount));
        return std::clamp(result, 0.0f, 1.0f);
    }
}

inline void shadows(std::vector<float>& image, float value) {

    auto applyShadows = [&](float& red, float& green, float& blue) {
        red = shadowsFunction(red, value);
        green = shadowsFunction(green, value);
        blue = shadowsFunction(blue, value);
    };

    iterateImageMutableMultiThread(image, applyShadows);
    return;
};