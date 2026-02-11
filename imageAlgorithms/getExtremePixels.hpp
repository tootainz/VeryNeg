#pragma once

#include <tuple>
#include <vector>

#include "iterateImage.hpp"

std::tuple<float, float, float> getMaxPixel(std::vector<float>& image, bool measureBrightest) {

    std::tuple<float, float, float> max(image[0],image[1],image[2]);
    float maxAverage = (std::get<0>(max) + std::get<1>(max) + std::get<2>(max))/3.0;

    auto compareValue = [&max, &maxAverage, &measureBrightest](float red, float green, float blue) {
        float currentAverage = (red + green + blue)/3.0;
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

std::tuple<float, float, float> getBrightestPixel(std::vector<float>& image) {
    return getMaxPixel(image, true);
}

std::tuple<float, float, float> getDarkestPixel(std::vector<float> image) {
    return getMaxPixel(image, false);
}