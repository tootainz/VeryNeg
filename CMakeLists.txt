cmake_minimum_required(VERSION 3.24)

# create project
project("VeryNeg" VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 23)

# add executable
add_executable(VeryNeg main.cpp
  Controller/Controller.cpp
  Negative/Negative.cpp
  Model/Model.cpp
  View/View.cpp
  rmlui-backend/RmlUi_Backend_SFML_GL2.cpp
)

# ------------------------------------------------
# Copy static assets

# Create a target that copies the assets folder
add_custom_target(CopyAssets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:VeryNeg>/assets"
    COMMENT "Copying assets to build folder"
)

# Make sure VeryNeg depends on the assets target
add_dependencies(VeryNeg CopyAssets)

# ------------------------------------------------
# Add libraries

include(FetchContent)

find_package(OpenGL REQUIRED)
find_package(OpenImageIO REQUIRED)

# Set properties that will be used while building RMLUI
set(RMLUI_BACKEND "SFML_GL2" CACHE STRING "" FORCE)

FetchContent_Declare(
    RmlUi
    GIT_REPOSITORY https://github.com/mikke89/RmlUi.git
    GIT_TAG 6.2
)
FetchContent_MakeAvailable(RmlUi)

FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_MakeAvailable(json)

add_library(rmlui_sfml STATIC
  libraries/rmlui-backends/RmlUi_Platform_SFML.cpp
  libraries/rmlui-backends/RmlUi_Renderer_GL2.cpp
)
target_include_directories(rmlui_sfml
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libraries/rmlui-backends
)
target_link_libraries(rmlui_sfml PRIVATE RmlUi::RmlUi SFML::Graphics)

# ------------------------------------------------

# link dependencies
target_link_libraries(VeryNeg PRIVATE OpenImageIO::OpenImageIO SFML::Graphics nlohmann_json::nlohmann_json RmlUi::RmlUi rmlui_sfml ${OPENGL_LIBRARIES})