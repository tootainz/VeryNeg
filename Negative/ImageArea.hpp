#pragma once

#include <tuple>


// Utility struct for passing around rectangular areas of images for cropping or selecting etc.
struct ImageArea {
    int left;
    int top;
    int right;
    int bottom;
};