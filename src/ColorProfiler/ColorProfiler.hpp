#pragma once

#include <vector>
#include <optional>

#include <lcms2.h>

class ColorProfiler {
private:

    cmsHPROFILE displayProfile;
    cmsHPROFILE adobeRGB;
    cmsHPROFILE sRGB;
    cmsHPROFILE grayGamma22;

    cmsHTRANSFORM adobeToDisplayTransform;
    cmsHTRANSFORM adobeToSRGBTransform;
    cmsHTRANSFORM gray22ToDisplayTransform;

    bool wasConstructed;
    bool hasDisplayProfile; // tells if there is actually a display profile or if it points to sRGB profile

public:

    ColorProfiler();
    ~ColorProfiler();

    bool toAdobeRGB(std::vector<float>& image, const std::optional<std::vector<uint8_t>>& iccProfile);
    void adobeToSRGB(std::vector<float>& image);
    void adobeToDisplay(std::vector<uint8_t>& image);
    bool toGrayGamma22(std::vector<float>& image, const std::optional<std::vector<uint8_t>>& iccProfile);
    void grayGamma22ToDisplay(std::vector<uint8_t>& image);

    std::vector<uint8_t> getSRGB();
    std::vector<uint8_t> getAdobeRGB();
    std::vector<uint8_t> getGrayGamma22();

    bool getWasConstructed();
};