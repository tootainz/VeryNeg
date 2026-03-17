#pragma once

#include "iterateImage.hpp"


inline void crudeInversion(std::vector<float>& image) {
    auto applyInvert = [&](float& red, float& green, float& blue) {
        red = 1.0f-red;
        green = 1.0f-green;
        blue = 1.0f-blue;
    };
    iterateImageMutableMultiThread(image, applyInvert);
    return;
}