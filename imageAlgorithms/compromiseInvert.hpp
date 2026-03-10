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

inline void compromiseInvert(std::vector<float>& image, float minDensity, float maxDensity) {
    std::println("inverting the image with the compromise invert");

    const float maxExposure = logToExposure(charCurve(maxDensity, 1.0f, 0.0f, 1.0f));
    const float minExposure = logToExposure(charCurve(minDensity, 1.0f, 0.0f, 1.0f));
    
    auto applyCompromiseInvert = [&](float& red, float& green, float& blue) {

        // Convert the current pixels using the characteristic curves
        float convertedRed = logToExposure(charCurve(scanToDensity(red), 1.0f, 0.0f, 1.0f));
        float convertedGreen = logToExposure(charCurve(scanToDensity(green), 1.0f, 0.0f, 1.0f));
        float convertedBlue = logToExposure(charCurve(scanToDensity(blue), 1.0f, 0.0f, 1.0f));
        
        // Normalize the values to 0.0-1.0
        float normalizedRed = (convertedRed - minExposure) / (maxExposure - minExposure);
        float normalizedGreen = (convertedGreen - minExposure) / (maxExposure - minExposure);
        float normalizedBlue = (convertedBlue - minExposure) / (maxExposure - minExposure);

        red = normalizedRed;
        green = normalizedGreen;
        blue = normalizedBlue;

        // red = std::clamp(normalizedRed, 0.0f, 1.0f);
        // green = std::clamp(normalizedGreen, 0.0f, 1.0f);
        // blue = std::clamp(normalizedBlue, 0.0f, 1.0f);
    };
    
    iterateImageMutable(image, applyCompromiseInvert);
}