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
    bool hasDisplayProfile; // tells if there is actually a display profile or if it points to sRGB profile

public:

    ColorProfiler();
    ~ColorProfiler();

    bool toAdobeRGB(std::vector<float>& image, const std::optional<std::vector<uint8_t>>& iccProfile);
    void adobeToSRGB(std::vector<float>& image);
    void adobeToDisplay(std::vector<uint8_t>& image);

    std::vector<uint8_t> getSRGB();
    std::vector<uint8_t> getAdobeRGB();

    bool getWasConstructed();
};