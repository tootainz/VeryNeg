# triplet for compiling vcpkg libraries for arm macs and min macos version 11.0

set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CMAKE_SYSTEM_NAME Darwin)
set(VCPKG_OSX_DEPLOYMENT_TARGET "11.0")
set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_LIBRARY_LINKAGE "static")
set(VCPKG_CRT_LINKAGE "dynamic")