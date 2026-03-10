#pragma once

#include <vector>
#include <print>
#include <algorithm>


inline std::vector<double> makeIntegralImage(std::vector<float>& image, int imageWidth, int imageHeight) {
    
    std::println("Generating an integral image");
    
    std::vector<double> integralImage(imageWidth*imageHeight*3);

    for (int pixel = 0; pixel < integralImage.size()/3; pixel++) {

        int y = pixel/imageWidth;
        int x = pixel - imageWidth * y;
        
        // Repeat for all RGB channels
        for (int channel = 0; channel < 3; channel++) {

            double thisValue = image[pixel*3+channel];
            double aboveValue;
            double leftValue;
            double aboveLeftValue;
            
            // first row
            if (y == 0) {
                aboveValue = 0;
                aboveLeftValue = 0;  
            } else {
                aboveValue = integralImage[(pixel-imageWidth)*3+channel];
                if (x == 0) {
                    aboveLeftValue = 0;
                } else {
                    aboveLeftValue = integralImage[((pixel-1)-imageWidth)*3+channel];
                }
            }

            // first column
            if (x == 0) {
                leftValue = 0;
            } else {
                leftValue = integralImage[(pixel-1)*3+channel];
            }


            // How to calculate the value of a single pixel
            // I(x,y) = i(x,y) + I(x,y-1) + I(x-1,y) - I(x-1,y-1)
            double newValue = thisValue + aboveValue + leftValue - aboveLeftValue;

            integralImage[pixel*3+channel] = newValue;
        }
    }
    return integralImage;
}

inline void boxFilter(std::vector<float>& image, int imageWidth, int imageHeight, int filterDiameter) {

    // Helper to access the image array easier
    auto xyToPixelIndex = [](int x, int y, int channel, int imageWidth) {
        return (x + y*imageWidth)*3 + channel;
    };
    
    std::println("performing a pro box blur with a diameter of: {}", filterDiameter);

    // Calculate the integral image
    std::vector<double> integralImage = makeIntegralImage(image, imageWidth, imageHeight);

    // Make sure that the diameter is odd
    int correctedFilterDiameter = filterDiameter;

    if (filterDiameter % 2 == 0) {
        correctedFilterDiameter += 1;
    }

    int filterRadius = correctedFilterDiameter/2;

    std::println("filtering");

    for (int pixel = 0; pixel < image.size()/3; pixel++) {

        int y = pixel/imageWidth;
        int x = pixel - imageWidth * y;

        // Clamp kernel bounds to image
        int left   = std::max(0, x - filterRadius);
        int right  = std::min(imageWidth  - 1, x + filterRadius);
        int top    = std::max(0, y - filterRadius);
        int bottom = std::min(imageHeight - 1, y + filterRadius);

        int filterArea = (right - left + 1) * (bottom - top + 1);

        // Convert to integral-image lookup coords
        int iLeft = left - 1;
        int iTop  = top  - 1;

        for (int channel = 0; channel < 3; channel++) {

            double a = 0.0;
            double b = 0.0;
            double c = 0.0;
            double d = 0.0;

            if (iLeft >= 0 && iTop >= 0)
                a = integralImage[xyToPixelIndex(iLeft, iTop, channel, imageWidth)];

            if (iTop >= 0)
                b = integralImage[xyToPixelIndex(right, iTop, channel, imageWidth)];

            if (iLeft >= 0)
                c = integralImage[xyToPixelIndex(iLeft, bottom, channel, imageWidth)];

            d = integralImage[xyToPixelIndex(right, bottom, channel, imageWidth)];

            // i(x,y) = I(D) + I(A) - I(B) - I(C)
            double filterSum = d - b - c + a;

            float filterAverage = filterSum / filterArea;

            image[pixel*3 + channel] = filterAverage;
        }
    }
}

inline void boxFilterSlow(std::vector<float>& image, int imageWidth, int imageHeight, int filterDiameter) {
    
    std::println("performing a box blur with a diameter of: {}", filterDiameter);

    // Make sure that the diameter is odd
    int correctedFilterDiameter = filterDiameter;

    if (filterDiameter % 2 == 0) {
        correctedFilterDiameter += 1;
    }

    int filterRadius = correctedFilterDiameter/2;

    // Keep a copy of the original image
    std::vector<float> originalImage = image;

    for (int pixel = 0; pixel < image.size()/3; pixel++) {

        int y = pixel/imageWidth;
        int x = pixel - imageWidth * y;

        float filterSumR = 0.0f;
        float filterSumG = 0.0f;
        float filterSumB = 0.0f;

        for (int yFilter = y - filterRadius; yFilter <= y + filterRadius; yFilter++) {
            for (int xFilter = x - filterRadius; xFilter <= x + filterRadius; xFilter ++) {

                // This is in case yFilter is out of bounds;
                int yFilterCorrected = yFilter;

                // Up top
                if (yFilter < 0) {
                    // Mirror the image
                    yFilterCorrected = -yFilter;
                // Down below
                } else if (yFilter >= imageHeight) {
                    // Mirror the image
                    yFilterCorrected = 2 * imageHeight - yFilter - 2;
                }

                // This is in case xFilter is out of bounds;
                int xFilterCorrected = xFilter;

                // Left
                if (xFilter < 0) {
                    // Mirror the image
                    xFilterCorrected = -xFilter;
                // Right
                } else if (xFilter >= imageWidth) {
                    // Mirror the image
                    xFilterCorrected = 2 * imageWidth - xFilter - 2;
                }

                int currentPixel = xFilterCorrected + yFilterCorrected*imageWidth;
                filterSumR += originalImage[currentPixel*3];
                filterSumG += originalImage[currentPixel*3 + 1];
                filterSumB += originalImage[currentPixel*3 + 2];
            }
        }

        float filterAverageR = filterSumR / (correctedFilterDiameter * correctedFilterDiameter);
        float filterAverageG = filterSumG / (correctedFilterDiameter * correctedFilterDiameter);
        float filterAverageB = filterSumB / (correctedFilterDiameter * correctedFilterDiameter);

        image[pixel*3] = filterAverageR;
        image[pixel*3 + 1] = filterAverageG;
        image[pixel*3 + 2] = filterAverageB;
    }
}