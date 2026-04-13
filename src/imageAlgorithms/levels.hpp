#pragma once

#include <tuple>
#include <vector>

#include "iterateImage.hpp"
#include "normalize.hpp"

// This is flawed atm
inline void levelsRGB(std::vector<float>& image, float originalMin, float originalMax, float targetMin, float targetMax) {
    auto applylevels = [&](float& red, float& green, float& blue) {
        red = normalize(red, originalMin, originalMax, targetMin, targetMax);
        green = normalize(green, originalMin, originalMax, targetMin, targetMax);
        blue = normalize(blue, originalMin, originalMax, targetMin, targetMax);
    };
    iterateImageMutableMultiThread(image, applylevels);
}

inline void levelsR(std::vector<float>& image, float originalMin, float originalMax, float targetMin, float targetMax) {
    auto applylevels = [&](float& red, float& green, float& blue) {
        red = normalize(red, originalMin, originalMax, targetMin, targetMax);
    };
    iterateImageMutableMultiThread(image, applylevels);
}

inline void levelsG(std::vector<float>& image, float originalMin, float originalMax, float targetMin, float targetMax) {
    auto applylevels = [&](float& red, float& green, float& blue) {
        green = normalize(green, originalMin, originalMax, targetMin, targetMax);
    };
    iterateImageMutableMultiThread(image, applylevels);
}

inline void levelsB(std::vector<float>& image, float originalMin, float originalMax, float targetMin, float targetMax) {
    auto applylevels = [&](float& red, float& green, float& blue) {
        blue = normalize(blue, originalMin, originalMax, targetMin, targetMax);
    };
    iterateImageMutableMultiThread(image, applylevels);
}