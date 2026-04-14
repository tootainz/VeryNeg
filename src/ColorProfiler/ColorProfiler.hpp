#pragma once

#include <vector>
#include <optional>

#include <lcms2.h>

class ColorProfiler {
private:

    cmsHPROFILE displayProfile;
    cmsHPROFILE adobeRGB;
    cmsHPROFILE sRGB;

    cmsHTRANSFORM displayTransform;
    cmsHTRANSFORM sRGBTransform;

    bool wasConstructed;

public:

    ColorProfiler();
    ~ColorProfiler();

    bool toAdobeRGB(std::vector<float>& image, int channelAmount, const std::optional<std::vector<uint8_t>>& iccProfile);
    void adobeToSRGB(std::vector<float>& image, int channelAmount);
    void adobeToDisplay(std::vector<uint8_t>& image, int channelAmount);

    std::vector<uint8_t> getSRGB();
    std::vector<uint8_t> getAdobeRGB();

    bool getWasConstructed();
};