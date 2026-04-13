#pragma once

#include <tuple>


// Utility struct for passing around rectangular areas of images for cropping or selecting etc.
struct ImageArea {
    
    // left and right are x coordinates
    // top and bottom are y coordinates
    int left;
    int top;
    int right;
    int bottom;
};