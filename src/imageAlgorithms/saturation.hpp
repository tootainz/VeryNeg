#pragma once
#include <algorithm>

// This is completely done by ChatGPT since saturation wasnt my main goal to develop and research
inline std::tuple<float,float,float> saturationFunction(float r, float g, float b, float saturation) {
    // Compute luminance (linear RGB)
    float L = 0.2126f * r + 0.7152f * g + 0.0722f * b;

    // Apply saturation
    float red = L + saturation * (r - L);
    float green = L + saturation * (g - L);
    float blue = L + saturation * (b - L);

    // Clamp to [0, 1]
    red = std::clamp(red, 0.0f, 1.0f);
    green = std::clamp(green, 0.0f, 1.0f);
    blue = std::clamp(blue, 0.0f, 1.0f);

    return {red, green, blue};
}

inline void saturation(std::vector<float>& image, float saturation) {
    auto applySaturation = [&](float& r, float& g, float& b) {
        auto [red, green, blue] = saturationFunction(r, g, b, saturation);
        r = red;
        g = green;
        b = blue;
    };

    iterateImageMutableMultiThread(image, applySaturation);
}