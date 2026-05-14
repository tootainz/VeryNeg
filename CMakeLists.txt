cmake_minimum_required(VERSION 3.24)

# create project
project("VeryNeg" VERSION 0.4.0)

set(CMAKE_CXX_STANDARD 23)

# # add executable
# add_executable(VeryNeg
#     src/main.cpp
#     src/Controller/Controller.cpp
#     src/Negative/Negative.cpp
#     src/Model/Model.cpp
#     src/View/View.cpp
#     src/RmlUi_Backend/RmlUi_Backend_SFML_GL2.cpp
#     src/ColorProfiler/ColorProfiler.cpp
# )

# add executable with MacOs .app bundle
add_executable(VeryNeg MACOSX_BUNDLE
    src/main.cpp
    src/Controller/Controller.cpp
    src/Negative/Negative.cpp
    src/Model/Model.cpp
    src/View/View.cpp
    src/RmlUi_Backend/RmlUi_Backend_SFML_GL2.cpp
    src/ColorProfiler/ColorProfiler.cpp
)

set_target_properties(VeryNeg PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist
)


# RESOURCES
# ------------------------------------------------

# # Create a target that copies the resources folder
# add_custom_target(CopyResources ALL
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#             "${CMAKE_SOURCE_DIR}/resources"
#             "$<TARGET_FILE_DIR:VeryNeg>/resources"
#     COMMENT "Copying resources to build folder"
# )

# # Make sure VeryNeg depends on the resources target
# add_dependencies(VeryNeg CopyResources)


# LIBRARIES
# ------------------------------------------------

# OpenGL
find_package(OpenGL REQUIRED)

# OIIO
find_package(OpenImageIO CONFIG REQUIRED)

# Lcms
find_package(lcms2 CONFIG REQUIRED)

# Json
find_package(nlohmann_json CONFIG REQUIRED)

# RmlUi
set(RMLUI_BACKEND "SFML_GL2" CACHE STRING "" FORCE)
find_package(RmlUi CONFIG REQUIRED)

# Add SFML from vcpkg if no need to edit the source
# find_package(SFML COMPONENTS Graphics Window System CONFIG REQUIRED)

# Add SFML as a submodule for editing the source

# SFML build options
set(BUILD_SHARED_LIBS OFF)
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_TEST_SUITE OFF CACHE BOOL "" FORCE)

set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)

set(SFML_USE_SYSTEM_DEPS ON CACHE BOOL "" FORCE)

add_subdirectory(libraries/SFML-3.1.0)

# RmlUi_sfml_backend
add_library(rmlui_sfml STATIC
  libraries/rmlui-backends/RmlUi_Platform_SFML.cpp
  libraries/rmlui-backends/RmlUi_Renderer_GL2.cpp
)
target_include_directories(rmlui_sfml
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libraries/rmlui-backends
)
target_link_libraries(rmlui_sfml PRIVATE RmlUi::RmlUi SFML::Graphics)


# LINK
# ------------------------------------------------

target_link_libraries(VeryNeg PRIVATE
    OpenImageIO::OpenImageIO
    SFML::Graphics
    nlohmann_json::nlohmann_json
    RmlUi::RmlUi
    lcms2::lcms2
    rmlui_sfml
    ${OPENGL_LIBRARIES}
)

if(APPLE)
    target_link_libraries(VeryNeg PRIVATE
        "-framework CoreGraphics"
        "-framework ApplicationServices"
        "-framework CoreFoundation"
    )
endif()


# INSTALL
# ------------------------------------------------

# Install app bundle
install(TARGETS VeryNeg
    BUNDLE DESTINATION .
    RUNTIME DESTINATION VeryNeg.app/Contents/MacOS
)

# Resources
install(DIRECTORY "${CMAKE_SOURCE_DIR}/resources/"
    DESTINATION VeryNeg.app/Contents/Resources
)

# cmake --install out/build/default_release --prefix out/install