cmake_minimum_required(VERSION 3.24)

project(VeryNeg VERSION 0.5.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------
# EXECUTABLE
# ------------------------------------------------

add_executable(VeryNeg
    src/main.cpp
    src/Controller/Controller.cpp
    src/Negative/Negative.cpp
    src/Model/Model.cpp
    src/View/View.cpp
    src/RmlUi_Backend/RmlUi_Backend_SFML_GL2.cpp
    src/ColorProfiler/ColorProfiler.cpp
)

# ------------------------------------------------
# MACOS
# ------------------------------------------------

if(APPLE)
    set_target_properties(VeryNeg PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST
            "${CMAKE_SOURCE_DIR}/Info.plist"
    )

    # target for which OS version is supported
    # set(CMAKE_OSX_DEPLOYMENT_TARGET "10.3" CACHE STRING "" FORCE)
endif()

# ------------------------------------------------
# WINDOWS
# ------------------------------------------------

if(WIN32)
    target_sources(VeryNeg PRIVATE
        VeryNeg.rc
    )
endif()

# ------------------------------------------------
# LIBRARIES
# ------------------------------------------------

find_package(OpenGL REQUIRED)
find_package(OpenImageIO CONFIG REQUIRED)
find_package(lcms2 CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

set(RMLUI_BACKEND "SFML_GL2" CACHE STRING "" FORCE)
find_package(RmlUi CONFIG REQUIRED)

# SFML
set(BUILD_SHARED_LIBS OFF)
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_TEST_SUITE OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
set(SFML_USE_SYSTEM_DEPS ON CACHE BOOL "" FORCE)

add_subdirectory(libraries/SFML-3.1.0)

# RmlUi SFML backend
add_library(rmlui_sfml STATIC
    libraries/rmlui-backends/RmlUi_Platform_SFML.cpp
    libraries/rmlui-backends/RmlUi_Renderer_GL2.cpp
)

target_include_directories(rmlui_sfml
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/libraries/rmlui-backends"
)

target_link_libraries(rmlui_sfml
    PRIVATE
        RmlUi::RmlUi
        SFML::Graphics
)

# ------------------------------------------------
# LINK
# ------------------------------------------------

target_link_libraries(VeryNeg
    PRIVATE
        OpenImageIO::OpenImageIO
        SFML::Graphics
        nlohmann_json::nlohmann_json
        RmlUi::RmlUi
        lcms2::lcms2
        rmlui_sfml
        ${OPENGL_LIBRARIES}
)

# ------------------------------------------------
# PLATFORM-SPECIFIC LIBRARIES
# ------------------------------------------------

if(APPLE)
    target_link_libraries(VeryNeg PRIVATE
        "-framework CoreGraphics"
        "-framework ApplicationServices"
        "-framework CoreFoundation"
    )
elseif(WIN32)
    target_link_libraries(VeryNeg PRIVATE
        Gdi32
        User32
    )
endif()

# ------------------------------------------------
# INSTALL
# ------------------------------------------------

if(APPLE)

    install(TARGETS VeryNeg
        BUNDLE DESTINATION .
        RUNTIME DESTINATION VeryNeg.app/Contents/MacOS
    )

    install(DIRECTORY
        "${CMAKE_SOURCE_DIR}/resources/"
        DESTINATION VeryNeg.app/Contents/Resources
    )

    # Use this command for installing the app bundle on MacOS
    # cmake --install out/build/mac_intel_release --prefix out/install/mac_intel_release
    # cmake --install out/build/mac_arm_release --prefix out/install/mac_arm_release

elseif(WIN32)

    install(TARGETS VeryNeg
        RUNTIME DESTINATION .
    )

    install(DIRECTORY
        "${CMAKE_SOURCE_DIR}/resources/"
        DESTINATION resources
    )

endif()