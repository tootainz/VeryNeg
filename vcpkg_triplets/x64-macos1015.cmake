# triplet for compiling vcpkg libraries for intel macs and min macos version 10.15

set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CMAKE_SYSTEM_NAME Darwin)
set(VCPKG_OSX_DEPLOYMENT_TARGET "10.15")
set(VCPKG_OSX_ARCHITECTURES x86_64)
set(VCPKG_LIBRARY_LINKAGE "static")
set(VCPKG_CRT_LINKAGE "dynamic")