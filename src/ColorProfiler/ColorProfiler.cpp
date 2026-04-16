#include "ColorProfiler.hpp"

#include <print>
#include <optional>

#include "getDisplayProfile.hpp"


// Helper written by AI to check if a profile is AdobeRGB
static bool isAdobeRGB(cmsHPROFILE profile) {
    if (!profile) return false;

    char desc[256] = {0};

    cmsGetProfileInfoASCII(
        profile,
        cmsInfoDescription,
        "en",
        "US",
        desc,
        sizeof(desc)
    );
    std::println("found profile metadata: {}", desc);
    return std::string(desc).find("Adobe RGB (1998)") != std::string::npos;
}

// Constructor
ColorProfiler::ColorProfiler() {

    // Initialize everything to null
    this->sRGB = nullptr;
    this->adobeRGB = nullptr;
    this->displayProfile = nullptr;
    this->displayTransform = nullptr;
    this->sRGBTransform = nullptr;

    // Get the profiles and open them

    // Open AdobeRGB profile
    this->adobeRGB = cmsOpenProfileFromFile("./resources/iccProfiles/AdobeRGB1998.icc", "r");
    if (!adobeRGB) {
        std::println("failed to open AdobeRGB profile");
        // failed to open the adobeRGB profile
        this->wasConstructed = false;
        return;
    }

    this->sRGB = cmsCreate_sRGBProfile();

    // Get the display profile from the system, if not specified, use sRGB
    std::optional<std::vector<uint8_t>> displayOptional = getDisplayProfile();
    if (displayOptional) {
        std::println("there is a display profile");
        this->displayProfile = cmsOpenProfileFromMem(displayOptional->data(), displayOptional->size());
        this->hasDisplayProfile = true;
        if (!displayProfile) {
            std::println("failed to open display profile");
            // failed to open the display profile
            this->wasConstructed = false;
            return;
        }
    }
    else {
        std::println("there is no display profile, defaulting to sRGB");
        this->displayProfile = this->sRGB;
        this->hasDisplayProfile = false;
    }

    // Create the transformations

    // Create the AdobeRgb to Display transform
    this->displayTransform = cmsCreateTransform(
        this->adobeRGB,
        TYPE_RGBA_8,
        this->displayProfile,
        TYPE_RGBA_8,
        INTENT_RELATIVE_COLORIMETRIC,
        0
    );
    if (!this->displayTransform) {
        // Something went wrong
        std::println("Something went wrong creating display transformation");
        this->wasConstructed = false;
        return;
    }

    // Create the AdobeRGB to sRGB profile transform
    this->sRGBTransform = cmsCreateTransform(
        this->adobeRGB,
        TYPE_RGB_FLT,
        this->sRGB,
        TYPE_RGB_FLT,
        INTENT_RELATIVE_COLORIMETRIC,
        0
    );
    if (!this->sRGBTransform) {
        std::println("somethign went wrong initing the sRGB transform");
        this->wasConstructed = false;
        return;
    }

    this->wasConstructed = true;
    return;
}

// Destructor
ColorProfiler::~ColorProfiler() {
    cmsCloseProfile(this->adobeRGB);
    cmsCloseProfile(this->sRGB);
    if (this->hasDisplayProfile) {
        cmsCloseProfile(this->displayProfile);
    }
    cmsDeleteTransform(this->displayTransform);
    cmsDeleteTransform(this->sRGBTransform);
}

// Converts from the provided icc blob to AdobeRGB
// Assumes that the data is sRGB if no profile is provided
// Returns true of the conversion was succesful or not performed due to missing or inclomplete icc profile
// Returns false if something went wrong in the conversion
bool ColorProfiler::toAdobeRGB(std::vector<float>& image, const std::optional<std::vector<uint8_t>>& iccProfile) {

    std::println("converting from input profile to adobe rgb");
    // This is practically C, since LCMS is written in C

    cmsHPROFILE hInProfile = nullptr;
    bool usingSRGB = false;

    // check if the iccProfile actually contains a profile
    if (iccProfile) {
        // Open the profile from the stored blob
        hInProfile = cmsOpenProfileFromMem(iccProfile->data(), iccProfile->size());
        if (!hInProfile) {
            // failed to open the embedded profile
            // Assume that the profile is sRGB
            std::println("failed to open embedded profile, assuming the image is in sRGB");
            hInProfile = this->sRGB;
            usingSRGB = true;
        }
        if (isAdobeRGB(hInProfile)) {
            // The embedded profile is already AdobeRGB
            std::println("image is already in adobeRGB");
            return true;
        }
    }
    else {
        // There is no embedded profile, default to sRGB
        std::println("there is no embedded profile, assuming the image is in sRGB");
        usingSRGB = true;
        hInProfile = this->sRGB;
    }

    // Create the actual profile transform
    cmsHTRANSFORM hTransform = nullptr;
    hTransform = cmsCreateTransform(
        hInProfile,
        TYPE_RGB_FLT,
        this->adobeRGB,
        TYPE_RGB_FLT,
        INTENT_RELATIVE_COLORIMETRIC,
        0
    );

    if (!hTransform) {
        // Something went wrong
        std::println("Something went wrong in the transformation");
        if (!usingSRGB) {
            cmsCloseProfile(hInProfile);
        }
        return false;
    }

    // Close the profile if we are not using sRGB profile that should be left open
    if (!usingSRGB) {
        cmsCloseProfile(hInProfile);
    }

    std::println("transforming icc profiles");
    // Perform the actual transformation
    cmsDoTransform(
        hTransform,
        image.data(),
        image.data(),
        image.size()/3
    );

    // Free the transform
    cmsDeleteTransform(hTransform);
    return true;
}

// Converts from working space AdobeRGB to sRGB for display or export
// Returns true if the conversion was succesful
// Returns false if something went wrong in the conversion
void ColorProfiler::adobeToSRGB(std::vector<float>& image) {

    std::println("converting from Adobe RGB to sRGB");
    // Perform the actual transformation
    cmsDoTransform(
        this->sRGBTransform,
        image.data(),
        image.data(),
        image.size()/3
    );
}

// This assumes that the image is in 8 bits already and has 4 channels RGBA
void ColorProfiler::adobeToDisplay(std::vector<uint8_t> &image){
    
    std::println("converting from Adobe RGB to display");
    // Perform the actual transformation
    cmsDoTransform(
        this->displayTransform,
        image.data(),
        image.data(),
        image.size()/4
    );
}

std::vector<uint8_t> ColorProfiler::getSRGB() {
    cmsUInt32Number size = 0;

    if (!cmsSaveProfileToMem(this->sRGB, nullptr, &size))
        return {};

    std::vector<uint8_t> buffer(size);

    cmsSaveProfileToMem(this->sRGB, buffer.data(), &size);

    return buffer;
}

std::vector<uint8_t> ColorProfiler::getAdobeRGB() {
    cmsUInt32Number size = 0;

    if (!cmsSaveProfileToMem(this->adobeRGB, nullptr, &size))
        return {};

    std::vector<uint8_t> buffer(size);

    cmsSaveProfileToMem(this->adobeRGB, buffer.data(), &size);

    return buffer;
}

bool ColorProfiler::getWasConstructed() {
    return this->wasConstructed;
}
