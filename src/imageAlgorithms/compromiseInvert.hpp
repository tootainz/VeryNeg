#pragma once

#include <vector>
#include <cmath>
#include <print>

#include "iterateImage.hpp"
#include "EditChannel.hpp"
#include "normalize.hpp"


// HELPERS

inline float scanToDensity(float scanInput) {
    return std::log10(1/scanInput);
}

inline float densityToScan(float density) {
    return std::pow(10.0f, -density);
}

inline float logToExposure(float logExposure) {
    return std::pow(10.0f, logExposure);
}


// CHARACTERISTIC CURVE ESTIMATION USED FOR THE CONVERSION

// Possible to add a more sophisticated curve in the future or to edit this one if needed
inline float charCurve(float density, float slope, float xPos, float yPos) {
    return slope * (density - xPos) + yPos;
}


// THE ACTUAL INVERSION FUNCTION

inline void compromiseInvert(std::vector<float>& image, float minDensity, float maxDensity) {
    std::println("inverting the image with the compromise invert");

    const float maxExposure = logToExposure(charCurve(maxDensity, 1.0f, 0.0f, 1.0f));
    const float minExposure = logToExposure(charCurve(minDensity, 1.0f, 0.0f, 1.0f));
    
    // Lamda function that will be perfomed on each pixel of the image
    auto applyCompromiseInvert = [&](float& red, float& green, float& blue) {

        // Convert the current pixels using the characteristic curves
        float convertedRed = logToExposure(charCurve(scanToDensity(red), 1.0f, 0.0f, 1.0f));
        float convertedGreen = logToExposure(charCurve(scanToDensity(green), 1.0f, 0.0f, 1.0f));
        float convertedBlue = logToExposure(charCurve(scanToDensity(blue), 1.0f, 0.0f, 1.0f));

        // Normalize to 0-1
        float normalizedRed = normalize(convertedRed, minExposure, maxExposure, 0.001f, 0.999f);
        float normalizedGreen = normalize(convertedGreen, minExposure, maxExposure, 0.001f, 0.999f);
        float normalizedBlue = normalize(convertedBlue, minExposure, maxExposure, 0.001f, 0.999f);

        red = normalizedRed;
        green = normalizedGreen;
        blue = normalizedBlue;

        // // This is just for trying without any normalization
        // red = convertedRed;
        // green = convertedGreen;
        // blue = convertedBlue;
    };
    
    // perform the inversion for each pixel of the image
    iterateImageMutableMultiThread(image, applyCompromiseInvert);
}