#include <filesystem>
#include <cstdlib>

std::filesystem::path getCacheDir()
{
#ifdef __APPLE__
    const char* home = std::getenv("HOME");
    return std::filesystem::path(home) / "Library/Caches/VeryNeg";

#elif _WIN32
    const char* local = std::getenv("LOCALAPPDATA");
    return std::filesystem::path(local) / "VeryNeg/Cache";

#else // Linux
    const char* home = std::getenv("HOME");
    return std::filesystem::path(home) / ".cache/VeryNeg";
#endif
}