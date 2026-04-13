#pragma once

#include <vector>
#include <tuple>


// Utility struct for passing around preview/thumbnail images
struct ImageData {
    
    // Pixel data stored in a one dimensional array, where pixels are stored sequentially with their channels as well in the order of RGB. For example: [1R, 1G, 1B, 2R, 2G, 2B, ...]
    std::vector<uint8_t> data;
    int width;
    int height;
};