#pragma once

#ifdef __APPLE__
    #include <CoreFoundation/CoreFoundation.h>
    #include <limits.h>

inline std::string getResourcesFolderPath() {
    CFBundleRef bundle = CFBundleGetMainBundle();
    if (!bundle) return "";

    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(bundle);
    if (!resourcesURL) return "";

    char path[PATH_MAX];
    if (!CFURLGetFileSystemRepresentation(resourcesURL, true, (UInt8*)path, PATH_MAX)) {
        CFRelease(resourcesURL);
        return "";
    }

    CFRelease(resourcesURL);
    return std::string(path);
}

#endif

inline std::string getResourcesPath(const std::string& name) {
    
#ifdef __APPLE__

    std::string res = getResourcesFolderPath();
    if (!res.empty()) {
        return res + "/" + name;
    }

#endif

    // fallback for development
    return std::string("assets/") + name;
}