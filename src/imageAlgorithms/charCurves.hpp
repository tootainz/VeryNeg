/*

These are film characteristic curves for Kodak ColorPlus 200

The axes are
x: log exposure
y: density

Red
y = 1.530991*x^0 + 0.450229*x^1 + -0.110128*x^2 + -0.026213*x^3 + 0.002688*x^4 + -0.000581*x^5

return 1.530991 + 0.450229*x - 0.110128*pow(x,2) - 0.026213*pow(x,3) + 0.002688*pow(x,4) - 0.000581*pow(x,5);

Green
y = 2.003746*x^0 + 0.456389*x^1 + -0.11861*x^2 + -0.029558*x^3 + 0.000646*x^4 + -0.000982*x^5

return 2.003746 + 0.456389*x - 0.11861*pow(x,2) - 0.029558*pow(x,3) + 0.000646*pow(x,4) - 0.000982*pow(x,5);

Blue
y = 2.397699*x^0 + 0.518352*x^1 + -0.10075*x^2 + -0.044875*x^3 + -0.02177*x^4 + -0.006706*x^5

return 2.397699 + 0.518352*x - 0.10075*pow(x,2) - 0.044875*pow(x,3) - 0.02177*pow(x,4) - 0.006706*pow(x,5);

Then the density maths

density = log10(1 / transmittance)
and transmittance is what we get as the pixel value from the scan

*/

#pragma once

#include <vector>
#include <cmath>

#include "iterateImage.hpp"
#include "EditChannel.hpp"
#include "normalize.hpp"
#include "../debug_print.hpp"

enum class FilmStock {
    Gold_200,
    Vision3_250d
};

inline float scanToDensity(float scanInput) {
    return std::log10(1/scanInput);
}

inline float densityToScan(float density) {
    return std::pow(10.0f, -density);
}

inline float logToExposure(float logExposure) {
    return std::pow(10.0f, logExposure);
}

// The characteristic curves
inline float charCurveFunction(float x, FilmStock filmStock, EditChannel channel) {
        float blackCorrection = 0;

        if (filmStock == FilmStock::Gold_200) {
            if (channel == EditChannel::R) {
                // y = 1.530991*x^0 + 0.450229*x^1 + -0.110128*x^2 + -0.026213*x^3 + 0.002688*x^4 + -0.000581*x^5
                return 1.530991+blackCorrection + 0.450229*x - 0.110128*pow(x,2) - 0.026213*pow(x,3) + 0.002688*pow(x,4) - 0.000581*pow(x,5);
            } else if (channel == EditChannel::G) {
                // y = 2.003746*x^0 + 0.456389*x^1 + -0.11861*x^2 + -0.029558*x^3 + 0.000646*x^4 + -0.000982*x^5
                return 2.003746+blackCorrection + 0.456389*x - 0.11861*pow(x,2) - 0.029558*pow(x,3) + 0.000646*pow(x,4) - 0.000982*pow(x,5);
            } else if (channel == EditChannel::B) {
                // y = 2.397699*x^0 + 0.518352*x^1 + -0.10075*x^2 + -0.044875*x^3 + -0.02177*x^4 + -0.006706*x^5
                return 2.397699+blackCorrection + 0.518352*x - 0.10075*pow(x,2) - 0.044875*pow(x,3) - 0.02177*pow(x,4) - 0.006706*pow(x,5);
            } else { // channel is RGB
                return x; // you should never use RGB for this
            }
        } else if (filmStock == FilmStock::Vision3_250d) {
            if (channel == EditChannel::R) {
                // y = 1.502852*x^0 + 0.417178*x^1 + -0.076392*x^2 + -0.018149*x^3 + 0.002781*x^4 + 0.000239*x^5
                return 1.502852 + blackCorrection + 0.417178*x - 0.076392*pow(x,2) - 0.018149*pow(x,3) + 0.002781*pow(x,4) + 0.000239*pow(x,5);
            } else if (channel == EditChannel::G) {
                // y = 2.121248*x^0 + 0.502098*x^1 + -0.07292*x^2 + -0.019621*x^3 + 0.000766*x^4 + -0.000285*x^5
                return 2.121248 + blackCorrection + 0.502098*x - 0.07292*pow(x,2) - 0.019621*pow(x,3) + 0.000766*pow(x,4) - 0.000285*pow(x,5);
            } else if (channel == EditChannel::B) {
                // y = 2.341024*x^0 + 0.478954*x^1 + -0.082139*x^2 + -0.022744*x^3 + 3.4E-5*x^4 + -0.000386*x^5
                return 2.341024 + blackCorrection + 0.478954*x - 0.082139*pow(x,2) - 0.022744*pow(x,3) + 0.000034*pow(x,4) - 0.000386*pow(x,5);
            } else { // channel is RGB
                return x; // you should never use RGB for this
            }
        }
}

// Solving for y numerically function by ChatGPT
inline float charCurveInverse(float density, FilmStock filmStock, EditChannel channel) {
    // Note from human, these are the max and min x axis values from the characteristic curve data image from Kodak for kodak Gold
    float lo = -3.0;
    float hi = 1.0;

    // Clamp density to valid range
    float d_lo = charCurveFunction(lo, filmStock, channel);
    float d_hi = charCurveFunction(hi, filmStock, channel);

    if (density <= d_lo) return lo;
    if (density >= d_hi) return hi;

    for (int i = 0; i < 40; ++i) {
        float mid = 0.5f * (lo + hi);
        float d   = charCurveFunction(mid, filmStock, channel);

        if (d > density)
            hi = mid;
        else
            lo = mid;
    }

    return 0.5f * (lo + hi);
}

inline void charCurveInvert(std::vector<float>& image, float maxTransmission, float minTransmission) {
    DEBUG_PRINT("inverting the image with the char curves");

    const float maxExposure = logToExposure(charCurveInverse(scanToDensity(maxTransmission), FilmStock::Gold_200, EditChannel::B));
    const float minExposure = logToExposure(charCurveInverse(scanToDensity(minTransmission), FilmStock::Gold_200, EditChannel::R));

    auto applyCharCurve = [&](float& red, float& green, float& blue) {
        const float convertedRed = logToExposure(charCurveInverse(scanToDensity(red), FilmStock::Gold_200, EditChannel::R));
        const float convertedGreen = logToExposure(charCurveInverse(scanToDensity(green), FilmStock::Gold_200, EditChannel::G));
        const float convertedBlue = logToExposure(charCurveInverse(scanToDensity(blue), FilmStock::Gold_200, EditChannel::B));

        // Normalize to 0-1 or not quite fully, leaving a small margin
        float normalizedRed = normalize(convertedRed, minExposure, maxExposure, 0.001f, 0.999f);
        float normalizedGreen = normalize(convertedGreen, minExposure, maxExposure, 0.001f, 0.999f);
        float normalizedBlue = normalize(convertedBlue, minExposure, maxExposure, 0.001f, 0.999f);

        red = normalizedRed;
        green = normalizedGreen;
        blue = normalizedBlue;
    };

    iterateImageMutableMultiThread(image, applyCharCurve);
    return;
}