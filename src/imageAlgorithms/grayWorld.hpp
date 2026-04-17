#pragma once

#include <tuple>
#include <vector>
#include <cmath>
#include <print>

#include "iterateImage.hpp"
#include "EditChannel.hpp"
#include "multiply.hpp"
#include "gamma.hpp"


inline std::tuple<float, float, float> grayWorld(std::vector<float>& image) {

    std::println("Starting Gray World algorithm");

    int pixelAmount = image.size()/3;

    std::println("Total amount of pixels per channel: {}", pixelAmount);

    double rSum = 0;
    double gSum = 0;
    double bSum = 0;

    auto countSums = [&](float red, float green, float blue) {
        rSum += red;
        gSum += green;
        bSum += blue;
    };

    iterateImageImmutableSingleThread(image, countSums);

    float rAverage = rSum/pixelAmount;
    float gAverage = gSum/pixelAmount; 
    float bAverage = bSum/pixelAmount;

    std::println("Average of r pixels: {}", rAverage);
    std::println("Average of g pixels: {}", gAverage);
    std::println("Average of b pixels: {}", bAverage);

    float targetGray = (rAverage + gAverage + bAverage)/3;

    std::println("Target gray: {}", targetGray);

    float rScaling = targetGray/rAverage;
    float gScaling = targetGray/gAverage;
    float bScaling = targetGray/bAverage;

    std::println("Scaling factor for r: {}", rScaling);
    std::println("Scaling factor for g: {}", gScaling);
    std::println("Scaling factor for b: {}", bScaling);

    return {rScaling, gScaling, bScaling};
}