#pragma once

#include <vector>
#include <functional>
#include <thread>


inline void iterateImageImmutableSingleThread(std::vector<float>& image, std::function<void(float, float, float)> operation) {

    for (int pixel = 0; pixel < image.size(); pixel += 3) {
        float red = image[pixel];
        float green = image[pixel+1];
        float blue = image[pixel+2];
        operation(red, green, blue);
    }
}

inline void iterateImageAreaImmutableSingleThread(std::vector<float>& image, std::function<void(float, float, float)> operation, int imageWidth, ImageArea area) {

    auto xyToPixelIndex = [](int x, int y, int channel, int imageWidth) -> int {
        return (x + y*imageWidth)*3 + channel;
    };

    for (int y = area.top; y <= area.bottom; ++y) {
        for (int x = area.left; x <= area.right; ++x) {
            int pixel = xyToPixelIndex(x, y, 0, imageWidth);
            float red = image[pixel];
            float green = image[pixel+1];
            float blue = image[pixel+2];
            operation(red, green, blue);
        }
    }
}

inline void iterateImageMutableSingleThread(std::vector<float>& image, std::function<void(float&, float&, float&)> operation) {
    
    for (int pixel = 0; pixel < image.size(); pixel += 3) {
        operation(image[pixel], image[pixel+1], image[pixel+2]);
    }
}

// LOL, below is an improved and parallelized versions of my code by chatGPT

inline void iterateImageImmutableMultiThread( const std::vector<float>& image, std::function<void(float, float, float)> operation) {

    const size_t pixelCount = image.size() / 3;
    const unsigned threadCount = std::thread::hardware_concurrency();
    const size_t chunkSize = pixelCount / threadCount;

    std::vector<std::thread> threads;
    threads.reserve(threadCount);

    for (unsigned t = 0; t < threadCount; ++t) {
        size_t start = t * chunkSize;
        size_t end = (t == threadCount - 1) ? pixelCount : start + chunkSize;

        threads.emplace_back([&, start, end]() {
            for (size_t p = start; p < end; ++p) {
                size_t i = p * 3;
                operation(image[i], image[i + 1], image[i + 2]);
            }
        });
    }

    for (auto& th : threads)
        th.join();
}

inline void iterateImageMutableMultiThread( std::vector<float>& image, std::function<void(float&, float&, float&)> operation) {
    
    const size_t pixelCount = image.size() / 3;
    const unsigned threadCount = std::thread::hardware_concurrency();
    const size_t chunkSize = pixelCount / threadCount;

    std::vector<std::thread> threads;
    threads.reserve(threadCount);

    for (unsigned t = 0; t < threadCount; ++t) {
        size_t start = t * chunkSize;
        size_t end = (t == threadCount - 1) ? pixelCount : start + chunkSize;

        threads.emplace_back([&, start, end]() {
            for (size_t p = start; p < end; ++p) {
                size_t i = p * 3;
                operation(image[i], image[i + 1], image[i + 2]);
            }
        });
    }

    for (auto& th : threads)
        th.join();
}