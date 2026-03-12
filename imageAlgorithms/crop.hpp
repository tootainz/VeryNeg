#pragma once

#include <print>
#include <vector>
#include <tuple>

#include "iterateImage.hpp"
#include "../Negative/ImageArea.hpp"


inline std::tuple<std::vector<float>, int, int> crop(std::vector<float>& image, int width, int height, ImageArea area) {
    
    auto xyToPixelIndex = [](int x, int y, int channel, int imageWidth) -> int {
        return (x + y*imageWidth)*3 + channel;
    };

    auto pixelIndexToXY = [](int pixel, int imageWidth) -> std::tuple<int, int> {
        int y = pixel/imageWidth;
        int x = pixel - imageWidth * y;
        return {x, y};
    };

    int newWidth = area.right - area.left;
    int newHeight= area.bottom - area.top;

    std::vector<float> cropped;
    cropped.reserve(newWidth * newHeight * 3);

    for (int y = area.top; y < area.bottom; ++y) {
        for (int x = area.left; x < area.right; ++x) {
            int pixel = xyToPixelIndex(x, y, 0, width);
            cropped.push_back(image[pixel]);
            cropped.push_back(image[pixel+1]);
            cropped.push_back(image[pixel+2]);
        }
    }

    return {cropped, newWidth, newHeight};
};