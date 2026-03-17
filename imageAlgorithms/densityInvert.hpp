#pragma once

#include "iterateImage.hpp"
#include "gamma.hpp"
#include <cmath>


inline float densityInvertFunction(float measuredTransmission) {
    return 1.0f/measuredTransmission;
}

inline void densityInvert(std::vector<float>& image) {
    auto applyInvert = [&](float& red, float& green, float& blue) {
        red = densityInvertFunction(red);
        green = 0.75f * pow(densityInvertFunction(green), 0.77f);
        blue = 0.55f * pow(densityInvertFunction(blue), 0.63f);
    };
    iterateImageMutableMultiThread(image, applyInvert);
    return;
}