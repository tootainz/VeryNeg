#pragma once

#include <tuple>
#include <vector>
#include <cmath>
#include <print>

#include "iterateImage.hpp"
#include "EditChannel.hpp"
#include "colorBalance.hpp"

// Returns the scaling factors for the color channels
inline std::tuple<float, float, float> neutralPatch(std::vector<float>& image, float sampleR, float sampleG, float sampleB) {
    std::println("Starting neutral patch algorithm");

    float targetGray = (sampleR + sampleG + sampleB)/3.0f;

    std::println("Target gray: {}", targetGray);

    float rScaling = targetGray/sampleR;
    float gScaling = targetGray/sampleG;
    float bScaling = targetGray/sampleB;

    std::println("Scaling factor for r: {}", rScaling);
    std::println("Scaling factor for g: {}", gScaling);
    std::println("Scaling factor for b: {}", bScaling);

    return {rScaling, gScaling, bScaling};
}