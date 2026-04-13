// Cross platform exe path helper written by ChatGPT

#pragma once

#include <string>
#include <stdexcept>

#if defined(_WIN32)
    #include <windows.h>
#elif defined(__linux__)
    #include <unistd.h>
    #include <limits.h>
#elif defined(__APPLE__)
    #include <mach-o/dyld.h>
    #include <limits.h>
#else
    #error "Unsupported platform"
#endif

inline std::string getExecutablePath() {
#if defined(_WIN32)
    char path[MAX_PATH];
    DWORD length = GetModuleFileNameA(NULL, path, MAX_PATH);
    if (length == 0)
        throw std::runtime_error("Failed to get executable path");
    return std::string(path, length);

#elif defined(__linux__)
    char path[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
    if (count == -1)
        throw std::runtime_error("Failed to get executable path");
    return std::string(path, count);

#elif defined(__APPLE__)
    char path[PATH_MAX];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) != 0)
        throw std::runtime_error("Buffer too small for executable path");
    return std::string(path);

#endif
}