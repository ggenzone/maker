# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

if(GIT_VERSION)
    set(PROJECT_VER ${GIT_VERSION})
else()
    set(PROJECT_VER "1.0.0-dev")
endif()

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
# "Trim" the build. Include the minimal set of components, main, and anything it depends on.
idf_build_set_property(MINIMAL_BUILD ON)
project(03-OTA VERSION ${PROJECT_VER})
