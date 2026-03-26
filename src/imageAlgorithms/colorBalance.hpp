#pragma once

#include <vector>
#include <cmath>

#include "iterateImage.hpp"
#include "EditChannel.hpp"


inline float rationalCurveFunction(float input, float gamma) {
    return input/(input+gamma*(1-input));
}

inline void colorBalance(std::vector<float>& image, float gamma, EditChannel channel) {
    auto applyColorBalance = [&](float& red, float& green, float& blue) {
        if (channel == EditChannel::RGB) {
            red = rationalCurveFunction(red, gamma);
            green = rationalCurveFunction(green, gamma);
            blue = rationalCurveFunction(blue, gamma);
        } else if (channel == EditChannel::R) {
            red = rationalCurveFunction(red, gamma);
        } else if (channel == EditChannel::G) {
            green = rationalCurveFunction(green, gamma);
        } else {
            blue = rationalCurveFunction(blue, gamma);
        }
    };
    iterateImageMutableMultiThread(image, applyColorBalance);
    return;
}