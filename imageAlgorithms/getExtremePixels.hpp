#pragma once

#include <tuple>
#include <vector>

#include "iterateImage.hpp"

enum class MaxChannel {
    RGB,
    R,
    G,
    B
};

std::tuple<float, float, float> getMaxPixel(std::vector<float>& image, bool measureBrightest, MaxChannel channel) {

    std::tuple<float, float, float> max(image[0],image[1],image[2]);
    float maxAverage;

    if (channel == MaxChannel::RGB) {
        maxAverage = (std::get<0>(max) + std::get<1>(max) + std::get<2>(max))/3.0;
    } else if (channel == MaxChannel::R) {
        maxAverage = std::get<0>(max);
    } else if (channel == MaxChannel::G) {
        maxAverage = std::get<1>(max);
    } else {
        maxAverage = std::get<2>(max);
    }

    auto compareValue = [&max, &maxAverage, &measureBrightest, &channel](float red, float green, float blue) {
        float currentAverage;
        
        if (channel == MaxChannel::RGB) {
            currentAverage = (red + green + blue)/3.0;
        } else if (channel == MaxChannel::R) {
            currentAverage = red;
        } else if (channel == MaxChannel::G) {
            currentAverage = green;
        } else {
            currentAverage = blue;
        }

        if (measureBrightest) {
            if (currentAverage > maxAverage) {
                maxAverage = currentAverage;
                max = {red, green, blue};
            }
        } else {
            if (currentAverage < maxAverage) {
                maxAverage = currentAverage;
                max = {red, green, blue};
            }
        }
    };

    iterateImageImmutable(image, compareValue);
    return max;
}

std::tuple<float, float, float> getBrightestPixel(std::vector<float>& image, MaxChannel channel) {
    return getMaxPixel(image, true, channel);
}

std::tuple<float, float, float> getDarkestPixel(std::vector<float> image, MaxChannel channel) {
    return getMaxPixel(image, false, channel);
}