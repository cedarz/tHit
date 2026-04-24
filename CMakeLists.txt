cmake_minimum_required(VERSION 3.20)
#set(CMAKE_TOOLCHAIN_FILE "D:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Toolchain file for vcpkg")
project(tHit LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# C++ version and std
if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>)
set_property(GLOBAL PROPERTY USE_FOLDERS TRUE)

# externals
add_subdirectory(externals)

# OpenGL
find_package(OpenGL REQUIRED)
# find_package(PNG REQUIRED)
# find_package(ZLIB REQUIRED)

include_directories(
    externals
)

# model-viewer
add_subdirectory(model-viewer)
