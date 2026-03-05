#pragma once

#include <vector>
#include <cmath>
#include <print>

#include "iterateImage.hpp"
#include "EditChannel.hpp"

inline float scanToDensity(float scanInput) {
    return std::log10(1/scanInput);
}

inline float densityToScan(float density) {
    return std::pow(10.0f, -density);
}

inline float charCurve(float density, float slope, float xPos, float yPos) {
    return slope * (density - xPos) + yPos;
}

inline float logToExposure(float logExposure) {
    return std::pow(10.0f, logExposure);
}

inline void compromiseInvert(std::vector<float>& image, float darkestDensity, float bSlope, float gSlope, float minDensity, float maxDensity) {
    std::println("inverting the image with the compromise invert");

    const float maxExposure = logToExposure(charCurve(maxDensity, 1.0f, 0.0f, -3.0f));
    const float minExposure = logToExposure(charCurve(minDensity, 1.0f, 0.0f, -3.0f));
    
    auto applyCompromiseInvert = [&](float& red, float& green, float& blue) {

        // Compute anchor points dynamically
        float redAnchor = charCurve(scanToDensity(red), 1.0f, 0.0f, -3.0f);
        float greenAnchor = charCurve(scanToDensity(green), 1.0f, 0.0f, -3.0f);
        float blueAnchor = charCurve(scanToDensity(blue), 1.0f, 0.0f, -3.0f);
        
        // Recalculate values with new slope, anchored at darkestDensity
        float adjustedRed = logToExposure(charCurve(scanToDensity(red), 1, darkestDensity, redAnchor));
        float adjustedGreen = logToExposure(charCurve(scanToDensity(green), gSlope, darkestDensity, greenAnchor));
        float adjustedBlue = logToExposure(charCurve(scanToDensity(blue), bSlope, darkestDensity, blueAnchor));

        red = adjustedRed;
        green = adjustedGreen;
        blue = adjustedBlue;

        // red = (adjustedRed - minExposure) / (maxExposure - minExposure);
        // green = (adjustedGreen - minExposure) / (maxExposure - minExposure);
        // blue = (adjustedBlue - minExposure) / (maxExposure - minExposure);
    };
    
    iterateImageMutable(image, applyCompromiseInvert);
}