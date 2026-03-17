cmake_minimum_required(VERSION 3.24)

# create project
project("VeryNeg" VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 23)

# add executable
add_executable(VeryNeg main.cpp
  Controller/Controller.cpp
  Negative/Negative.cpp
  Model/Model.cpp
  rmlui-backend/RmlUi_Backend_SFML_GL2.cpp
)

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

# Set properties that will be used while building TGUI
set(BUILD_SHARED_LIBS OFF)      # Determines whether TGUI build a static or dynamic/shared library
set(TGUI_BACKEND SFML_GRAPHICS)  # Sets which backend TGUI will use
set(TGUI_BUILD_GUI_BUILDER ON)  # Also compiles the gui builder

FetchContent_Declare(
  TGUI
  GIT_REPOSITORY https://github.com/texus/TGUI
  GIT_TAG v1.12.0  # Change this to wanted version
)
FetchContent_MakeAvailable(TGUI)

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
target_link_libraries(VeryNeg PRIVATE OpenImageIO::OpenImageIO SFML::Graphics TGUI::TGUI nlohmann_json::nlohmann_json RmlUi::RmlUi rmlui_sfml ${OPENGL_LIBRARIES})