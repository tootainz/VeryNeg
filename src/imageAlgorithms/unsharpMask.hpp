#pragma once

#include <vector>

#include "gaussianFilter.hpp"
#include "../debug_print.hpp"

inline void unsharpMask(std::vector<float>& image, int imageWidth, int imageHeight, float amount,  int filterDiameter = 5, int gaussianResolution = 3) {
   
   DEBUG_PRINT("performing unsharp mask with amount {} and diameter {}", amount, filterDiameter);

    // Create copy of the original image for makign the unsharp mask
    std::vector<float> mask = image;

    // Blur the image
    gaussianFilter(mask, imageWidth, imageHeight, filterDiameter, gaussianResolution);

    // Get the mask by subtracting the blurred image from the original
    for (int pixel = 0; pixel < image.size(); pixel++) {
        mask[pixel] = image[pixel] - mask[pixel];
    }

    // Perform the sharpening by adding the mask to the original image
    for (int pixel = 0; pixel < image.size(); pixel++) {
        image[pixel] = image[pixel] + amount*mask[pixel];
    }

    return;
}