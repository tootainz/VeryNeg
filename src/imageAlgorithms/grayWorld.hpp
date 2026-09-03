#pragma once

#include <tuple>
#include <vector>
#include <cmath>

#include "iterateImage.hpp"
#include "EditChannel.hpp"
#include "gamma.hpp"
#include "crop.hpp"
#include "../Negative/ImageArea.hpp"
#include "../debug_print.hpp"


inline std::tuple<float, float, float> grayWorld(std::vector<float>& image, int width, int height, ImageArea area) {

    DEBUG_PRINT("Starting Gray World algorithm");

    // Generate a copy of only the pixels indicated by the area
    auto [croppedImage, croppedWidth, croppedHeight] = crop(image, width, height, area);

    int pixelAmount = croppedImage.size()/3;

    DEBUG_PRINT("Total amount of pixels per channel: {}", pixelAmount);

    double rSum = 0;
    double gSum = 0;
    double bSum = 0;

    auto countSums = [&](float red, float green, float blue) {
        rSum += red;
        gSum += green;
        bSum += blue;
    };

    iterateImageImmutableSingleThread(croppedImage, countSums);

    float rAverage = rSum/pixelAmount;
    float gAverage = gSum/pixelAmount; 
    float bAverage = bSum/pixelAmount;

    DEBUG_PRINT("Average of r pixels: {}", rAverage);
    DEBUG_PRINT("Average of g pixels: {}", gAverage);
    DEBUG_PRINT("Average of b pixels: {}", bAverage);

    float targetGray = (rAverage + gAverage + bAverage)/3;

    DEBUG_PRINT("Target gray: {}", targetGray);

    float rScaling = targetGray/rAverage;
    float gScaling = targetGray/gAverage;
    float bScaling = targetGray/bAverage;

    // Set up my preferre neutral gray fro each channel by multiplying it
    // My preference: Red stays as the reference point, Green is a little bit less or almost the same as red, blue is noticeably less

    rScaling = rScaling;
    gScaling = gScaling * 0.98;
    bScaling = bScaling * 0.8;

    DEBUG_PRINT("Scaling factor for r: {}", rScaling);
    DEBUG_PRINT("Scaling factor for g: {}", gScaling);
    DEBUG_PRINT("Scaling factor for b: {}", bScaling);

    return {rScaling, gScaling, bScaling};
}