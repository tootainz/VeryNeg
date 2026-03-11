#pragma once

#include <tuple>
#include <vector>

#include "../Negative/ImageArea.hpp"
#include "iterateImage.hpp"

inline std::tuple<float, float, float> samplePixelArea(std::vector<float>& image, int imageWidth, int imageHeight, ImageArea area) {

    double rSum = 0.0;
    double gSum = 0.0;
    double bSum = 0.0;
    int pixelAmount = (area.right - area.left)*(area.bottom - area.top);

    auto sumPixels = [&rSum, &gSum, &bSum](float r, float g, float b) {
        rSum += r;
        gSum += g;
        bSum += b;
    };

    iterateImageAreaImmutableSingleThread(image, sumPixels, imageWidth, area);

    float rAverage = rSum/pixelAmount;
    float gAverage = gSum/pixelAmount;
    float bAverage = bSum/pixelAmount;

    return {rAverage, gAverage, bAverage};
}