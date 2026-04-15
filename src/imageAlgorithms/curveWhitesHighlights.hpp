#pragma once

#include <print>
#include <algorithm>
#include <cmath> 

#include "EditChannel.hpp"
#include "iterateImage.hpp"

// Whites
const float WHITES_DECREASE_DAMPENING = 0.1f;
const int WHITES_INCREASE_SHARPNESS = 40.0f;
const int WHITES_DECREASE_SHARPNESS = 5.0f;

inline float whitesFunction(float input, float value) {

    if (value >= 0.0f) {
        float amount = value;
        float absoluteInput = std::abs(input-1.0f);

        float result = 1.0f + (WHITES_INCREASE_SHARPNESS*(input-1.0f)) / ((WHITES_INCREASE_SHARPNESS-1.0f) + std::pow(absoluteInput, -amount));
        return std::clamp(result, 0.0f, 1.0f);
    }
    else {
        float amount = value*WHITES_DECREASE_DAMPENING;

        float base = input - 1.0f;
        // This signedpow stuff is really weird and i dont know why i have to do it.
        // Below is my original function commented out. For some reason a calculator understands it but c++ doesnt
        // So i need to do some randoms tuff that only ChatGPT could tell me
        // Anyways the weird signedpow function works

        // float result = 1.0f + (1.0f+amount)*(input - 1.0f) - amount*std::pow(input-1.0f, 1.0f/WHITES_DECREASE_SHARPNESS);
        // return std::clamp(result, 0.0f, 1.0f);

        float signedPow = std::copysign(std::pow(std::abs(base), 1.0f / WHITES_DECREASE_SHARPNESS), base);

        float result = 1.0f + (1.0f + amount) * base - amount * signedPow;

        return std::clamp(result, 0.0f, 1.0f);
    }
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

// Highlights
const float HIGHLIGHTS_INCREASE_DAMPENING = 0.1f;
const int HIGHLIGHTS_INCREASE_SHARPNESS = 9.0f;
const int HIGHLIGHTS_DECREASE_SHARPNESS = 15.0f;
const int HIGHLIGHTS_INCREASE_SLOPE = 0.08f;

inline float highlightsFunction(float input, float value) {

    if (value >= 0.0f) {
        float amount = value;
        float absoluteInput = std::abs(input-1.0f);

        float result = 1.0f + HIGHLIGHTS_INCREASE_SLOPE*(input-1.0f) + (1.0f-HIGHLIGHTS_INCREASE_SLOPE)*(HIGHLIGHTS_INCREASE_SHARPNESS*(input-1.0f)) / ((HIGHLIGHTS_INCREASE_SHARPNESS-1.0f) + std::pow(absoluteInput, -amount));
        return std::clamp(result, 0.0f, 1.0f);
    }
    else {
        float amount = value*HIGHLIGHTS_INCREASE_DAMPENING;

        float result = (1.0f+amount)*input - amount*std::pow(input, HIGHLIGHTS_DECREASE_SHARPNESS);
        return std::clamp(result, 0.0f, 1.0f);
    }
}

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