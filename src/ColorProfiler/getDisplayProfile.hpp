#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <optional>
#include <fstream>
#include <print>

#if defined(_WIN32)
    #include <windows.h>
    #include <wingdi.h>
#elif defined(__APPLE__)
    #include <ApplicationServices/ApplicationServices.h>
    #include <CoreGraphics/CoreGraphics.h>
#endif

// Gets the ICC profile of the main display. Please note that this does not support multiple displays!! Written by ChatGPT

// WINDOWS
#if defined(_WIN32)

inline std::optional<std::vector<uint8_t>> get_windows_display_icc()
{
    // Create a DC for the primary display.
    HDC hdc = GetDC(nullptr);
    if (!hdc)
        return std::nullopt;

    // First call asks Windows how large the filename buffer needs to be.
    DWORD size = 0;

    if (GetICMProfileW(hdc, &size, nullptr))
    {
        // Unexpected: normally the first call should fail with
        // ERROR_INSUFFICIENT_BUFFER.
        ReleaseDC(nullptr, hdc);
        return std::nullopt;
    }

    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER || size == 0)
    {
        ReleaseDC(nullptr, hdc);
        return std::nullopt;
    }

    std::vector<wchar_t> pathBuffer(size);

    if (!GetICMProfileW(hdc, &size, pathBuffer.data()))
    {
        ReleaseDC(nullptr, hdc);
        return std::nullopt;
    }

    ReleaseDC(nullptr, hdc);

    // GetICMProfileW returns the path to the ICC profile.
    std::filesystem::path profilePath(pathBuffer.data());

    std::ifstream file(profilePath, std::ios::binary | std::ios::ate);

    if (!file)
        return std::nullopt;

    const std::streamsize fileSize = file.tellg();

    if (fileSize <= 0)
        return std::nullopt;

    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> result(static_cast<size_t>(fileSize));

    if (!file.read(
            reinterpret_cast<char*>(result.data()),
            fileSize))
    {
        return std::nullopt;
    }

    std::println(
        "Got Windows ICC profile: {} bytes ({})",
        result.size(),
        profilePath.string());

    return result;
}

#endif


// MAC
#if defined(__APPLE__)

inline std::optional<std::vector<uint8_t>> get_macos_display_icc()
{
    CGDirectDisplayID display = CGMainDisplayID();

    CGColorSpaceRef colorSpace = CGDisplayCopyColorSpace(display);
    if (!colorSpace)
        return std::nullopt;

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


inline std::optional<std::vector<uint8_t>> getDisplayProfile()
{
#if defined(_WIN32)
    return get_windows_display_icc();
#elif defined(__APPLE__)
    return get_macos_display_icc();
#else
    return std::nullopt;
#endif
}