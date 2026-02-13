#pragma once

#include "iterateImage.hpp"
#include "gamma.hpp"


inline float densityInvertFunction(float measuredTransmission) {
    return 1.0f/measuredTransmission;
}

inline void densityInvert(std::vector<float>& image) {
    auto applyInvert = [&](float& red, float& green, float& blue) {
        red = densityInvertFunction(red);
        green = densityInvertFunction(green);
        blue = densityInvertFunction(blue);
    };
    iterateImageMutable(image, applyInvert);
    return;
}