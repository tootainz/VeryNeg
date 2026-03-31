#pragma once

#include <tuple>
#include <vector>

#include "../Negative/ImageArea.hpp"
#include "samplePixelArea.hpp"


inline std::tuple<float, float, float> eyedropper(std::vector<float>& image, int imageWidth, int imageHeight, int x, int y, int sampleSize) {

    int top = std::max(0, y - sampleSize/2);
    int bottom = std::min(imageHeight-1, y + sampleSize/2);
    int left = std::max(0, x - sampleSize/2);
    int right = std::min(imageWidth-1, x + sampleSize/2);

    return samplePixelArea(image, imageWidth, imageHeight, {left, top, right, bottom});
}