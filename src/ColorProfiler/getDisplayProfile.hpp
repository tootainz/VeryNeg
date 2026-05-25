#pragma once

#include <string>
#include <filesystem>
#include <optional>
#include <print>

#if defined(_WIN32)
    #include <windows.h>
    #include <icm.h>
#elif defined(__APPLE__)
    #include <ApplicationServices/ApplicationServices.h>
    #include <CoreGraphics/CoreGraphics.h>
#endif


// WINDOWS
#if defined(_WIN32)
    // windows stuff
#endif

// MAC
#if defined(__APPLE__)

#include <ApplicationServices/ApplicationServices.h>
#include <CoreGraphics/CoreGraphics.h>

inline std::optional<std::vector<uint8_t>> get_macos_display_icc() {
    CGDirectDisplayID display = CGMainDisplayID();

    // Get display color space
    CGColorSpaceRef colorSpace = CGDisplayCopyColorSpace(display);
    if (!colorSpace)
        return std::nullopt;

    // Extract ICC data from it
    CFDataRef iccData = CGColorSpaceCopyICCData(colorSpace);
    CGColorSpaceRelease(colorSpace);

    if (!iccData)
        return std::nullopt;

    const UInt8* bytes = CFDataGetBytePtr(iccData);
    CFIndex length = CFDataGetLength(iccData);

    if (!bytes || length <= 0)
    {
        CFRelease(iccData);
        return std::nullopt;
    }

    std::vector<uint8_t> result(bytes, bytes + length);

    CFRelease(iccData);

    std::println("Got ICC profile: {} bytes", result.size());

    return result;
}

#endif

std::optional<std::vector<uint8_t>> getDisplayProfile() {

    std::optional<std::vector<uint8_t>> profile;

    #if defined(_WIN32)
        profile = std::nullopt;
    #elif defined(__APPLE__)
        profile = get_macos_display_icc();
    #else
        profile = std::nullopt;
    #endif
    return profile;
}