#pragma once

#include "boxFilter.hpp"

#pragma once

#include "boxFilter.hpp"
#include <vector>
#include <cmath>

// Implemented with the help of ChatGPT since i was lazy

inline std::vector<int> computeBoxDiameters(float sigma, int n) {
    float wIdeal = std::sqrt((12.0f * sigma * sigma / n) + 1.0f);

    int wl = (int)std::floor(wIdeal);
    if (wl % 2 == 0) wl--;

    int wu = wl + 2;

    int m = (int)std::round(
        (12.0f * sigma * sigma -
         n * wl * wl -
         4 * n * wl -
         3 * n) /
        (-4.0f * wl - 4.0f)
    );

    std::vector<int> sizes;
    for (int i = 0; i < n; ++i) {
        sizes.push_back(i < m ? wl : wu);
    }

    return sizes;
}

inline void gaussianFilter( std::vector<float>& image, int imageWidth, int imageHeight, int filterDiameter, int resolution) {
    // Approximate sigma from diameter assuming Gaussian covers ~3σ each side
    float sigma = filterDiameter / 6.0f;

    std::vector<int> boxDiameters = computeBoxDiameters(sigma, resolution);

    for (int i = 0; i < resolution; ++i) {
        boxFilter(image, imageWidth, imageHeight, boxDiameters[i]);
    }
}