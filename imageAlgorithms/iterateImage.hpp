#pragma once

#include <tuple>
#include <vector>
#include <functional>

void iterateImageImmutable(std::vector<float>& image, std::function<void(float, float, float)> operation) {
    for (int pixel = 0; pixel < image.size(); pixel += 3) {
        float red = image[pixel];
        float green = image[pixel+1];
        float blue = image[pixel+2];
        operation(red, green, blue);
    }
}

void iterateImageMutable(std::vector<float>& image, std::function<void(float&, float&, float&)> operation) {
    for (int pixel = 0; pixel < image.size(); pixel += 3) {
        operation(image[pixel], image[pixel+1], image[pixel+2]);
    }
}