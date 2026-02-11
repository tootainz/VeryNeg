cmake_minimum_required(VERSION 3.24)

# create project
project("VeryNeg" VERSION 0.0.1)

# add executable
add_executable(VeryNeg main.cpp
    ./Negative/Negative.hpp
    ./imageAlgorithms/iterateImage.hpp
    ./imageAlgorithms/getExtremePixels.hpp
    ./imageAlgorithms/levels.hpp)

find_package(OpenImageIO REQUIRED)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

# link dependencies
target_link_libraries(VeryNeg PRIVATE OpenImageIO::OpenImageIO SFML::Graphics)