#pragma once

#include <vector>
#include <tuple>


// Utility struct for passing around preview/thumbnail images
struct ImageData {
    std::vector<uint8_t> data;
    int width;
    int height;
};