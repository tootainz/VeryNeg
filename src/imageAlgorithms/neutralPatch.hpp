#pragma once

#include <tuple>
#include <vector>
#include <cmath>

#include "iterateImage.hpp"
#include "EditChannel.hpp"
#include "colorBalance.hpp"
#include "../debug_print.hpp"

// Returns the scaling factors for the color channels
inline std::tuple<float, float, float> neutralPatch(float sampleR, float sampleG, float sampleB) {
    DEBUG_PRINT("Starting neutral patch algorithm");

    float targetGray = (sampleR + sampleG + sampleB)/3.0f;

    DEBUG_PRINT("Target gray: {}", targetGray);

    float rScaling = targetGray/sampleR;
    float gScaling = targetGray/sampleG;
    float bScaling = targetGray/sampleB;

    DEBUG_PRINT("Scaling factor for r: {}", rScaling);
    DEBUG_PRINT("Scaling factor for g: {}", gScaling);
    DEBUG_PRINT("Scaling factor for b: {}", bScaling);

    // Apply my interpretation of a neutral grey

    rScaling = rScaling;
    gScaling = gScaling * 0.99;
    bScaling = bScaling * 0.87;

    return {rScaling, gScaling, bScaling};
}