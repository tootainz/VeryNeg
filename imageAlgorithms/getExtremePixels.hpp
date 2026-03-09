#pragma once

#include <tuple>
#include <vector>

#include "iterateImage.hpp"
#include "EditChannel.hpp"

inline std::tuple<float, float, float> getMaxPixel(std::vector<float>& image, bool measureBrightest, EditChannel channel) {

    std::tuple<float, float, float> max(image[0],image[1],image[2]);
    float maxAverage;

    if (channel == EditChannel::RGB) {
        maxAverage = (std::get<0>(max) + std::get<1>(max) + std::get<2>(max))/3.0;
    } else if (channel == EditChannel::R) {
        maxAverage = std::get<0>(max);
    } else if (channel == EditChannel::G) {
        maxAverage = std::get<1>(max);
    } else {
        maxAverage = std::get<2>(max);
    }

    auto compareValue = [&max, &maxAverage, &measureBrightest, &channel](float red, float green, float blue) {
        float currentAverage;
        
        if (channel == EditChannel::RGB) {
            currentAverage = (red + green + blue)/3.0;
        } else if (channel == EditChannel::R) {
            currentAverage = red;
        } else if (channel == EditChannel::G) {
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

    iterateImageImmutableSingleThread(image, compareValue);
    return max;
}

inline std::tuple<float, float, float> getBrightestPixel(std::vector<float>& image, EditChannel channel) {
    return getMaxPixel(image, true, channel);
}

inline std::tuple<float, float, float> getDarkestPixel(std::vector<float> image, EditChannel channel) {
    return getMaxPixel(image, false, channel);
}