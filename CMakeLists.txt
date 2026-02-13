cmake_minimum_required(VERSION 3.24)

# create project
project("VeryNeg" VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 23)

# add executable
add_executable(VeryNeg main.cpp
  Controller/Controller.cpp
  Negative/Negative.cpp
)

# ------------------------------------------------
# Add libraries

find_package(OpenImageIO REQUIRED)

include(FetchContent)

FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

# Set properties that will be used while building TGUI
set(BUILD_SHARED_LIBS OFF)      # Determines whether TGUI build a static or dynamic/shared library
set(TGUI_BACKEND SFML_GRAPHICS)  # Sets which backend TGUI will use

FetchContent_Declare(
  TGUI
  GIT_REPOSITORY https://github.com/texus/TGUI
  GIT_TAG v1.12.0  # Change this to wanted version
)
FetchContent_MakeAvailable(TGUI)

# ------------------------------------------------

# link dependencies
target_link_libraries(VeryNeg PRIVATE OpenImageIO::OpenImageIO SFML::Graphics TGUI::TGUI)