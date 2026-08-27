#pragma once

#include <filesystem>
#include <string>

#ifdef _WIN32
    #include <windows.h>
#elif defined(__APPLE__)
    #include <CoreFoundation/CoreFoundation.h>
    #include <limits.h>
#endif


#ifdef _WIN32

inline std::filesystem::path getExecutableFolderPath()
{
    std::wstring buffer;
    DWORD size = 256;

    while (true)
    {
        buffer.resize(size);

        DWORD length = GetModuleFileNameW(
            nullptr,
            buffer.data(),
            size
        );

        if (length == 0)
            return {};

        if (length < size - 1)
        {
            buffer.resize(length);
            return std::filesystem::path(buffer).parent_path();
        }

        size *= 2;
    }
}

#elif defined(__APPLE__)

inline std::filesystem::path getResourcesFolderPath()
{
    CFBundleRef bundle = CFBundleGetMainBundle();
    if (!bundle)
        return {};

    CFURLRef resourcesURL =
        CFBundleCopyResourcesDirectoryURL(bundle);

    if (!resourcesURL)
        return {};

    char path[PATH_MAX];

    if (!CFURLGetFileSystemRepresentation(
            resourcesURL,
            true,
            reinterpret_cast<UInt8*>(path),
            PATH_MAX))
    {
        CFRelease(resourcesURL);
        return {};
    }

    CFRelease(resourcesURL);

    return std::filesystem::path(path);
}

#endif


inline std::filesystem::path getResourcesPath(
    const std::string& name)
{
#ifdef _WIN32

    const auto executableFolder = getExecutableFolderPath();

    if (!executableFolder.empty())
        return executableFolder / "resources" / name;

#elif defined(__APPLE__)

    const auto resourcesFolder = getResourcesFolderPath();

    if (!resourcesFolder.empty())
        return resourcesFolder / name;

#endif
}