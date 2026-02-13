#include "iterateImage.hpp"
#include "gamma.hpp"


float densityInvertFunction(float measuredTransmission) {
    return 1.0f/measuredTransmission;
}

void densityInvert(std::vector<float>& image) {
    auto applyInvert = [&](float& red, float& green, float& blue) {
        red = densityInvertFunction(red);
        green = densityInvertFunction(green);
        blue = densityInvertFunction(blue);
    };
    iterateImageMutable(image, applyInvert);
    return;
}