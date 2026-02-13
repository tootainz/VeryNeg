#pragma once

#include <tuple>
#include <vector>

#include "iterateImage.hpp"

inline float levels(float currentValue, float originalMin, float originalMax, float targetMin, float targetMax) {
    return(targetMin+((currentValue-originalMin)*(targetMax-targetMin))/(originalMax-originalMin));
}

// This is flawed atm
inline void levelsRGB(std::vector<float>& image, float originalMin, float originalMax, float targetMin, float targetMax) {
    auto applylevels = [&](float& red, float& green, float& blue) {
        red = levels(red, originalMin, originalMax, targetMin, targetMax);
        green = levels(green, originalMin, originalMax, targetMin, targetMax);
        blue = levels(blue, originalMin, originalMax, targetMin, targetMax);
    };
    iterateImageMutable(image, applylevels);
}

inline void levelsR(std::vector<float>& image, float originalMin, float originalMax, float targetMin, float targetMax) {
    auto applylevels = [&](float& red, float& green, float& blue) {
        red = levels(red, originalMin, originalMax, targetMin, targetMax);
    };
    iterateImageMutable(image, applylevels);
}

inline void levelsG(std::vector<float>& image, float originalMin, float originalMax, float targetMin, float targetMax) {
    auto applylevels = [&](float& red, float& green, float& blue) {
        green = levels(green, originalMin, originalMax, targetMin, targetMax);
    };
    iterateImageMutable(image, applylevels);
}

inline void levelsB(std::vector<float>& image, float originalMin, float originalMax, float targetMin, float targetMax) {
    auto applylevels = [&](float& red, float& green, float& blue) {
        blue = levels(blue, originalMin, originalMax, targetMin, targetMax);
    };
    iterateImageMutable(image, applylevels);
}