cmake_minimum_required(VERSION 3.20)

project(RTypeMapEditor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Use release version of raylib from buildtrees
set(RAYLIB_LIBRARY "/home/hugo/R-Type/vcpkg/buildtrees/raylib/x64-linux-rel/raylib/libraylib.a")
set(RAYLIB_INCLUDE_DIR "/home/hugo/R-Type/vcpkg/buildtrees/raylib/x64-linux-rel/raylib/include")
set(GLFW_LIBRARY "/home/hugo/R-Type/vcpkg/buildtrees/glfw3/x64-linux-rel/src/libglfw3.a")

# Verify paths exist
if(NOT EXISTS ${RAYLIB_LIBRARY})
    message(FATAL_ERROR "Raylib library not found at ${RAYLIB_LIBRARY}")
endif()

if(NOT EXISTS ${RAYLIB_INCLUDE_DIR}/raylib.h)
    message(FATAL_ERROR "Raylib headers not found at ${RAYLIB_INCLUDE_DIR}")
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/map_editor.cpp
    src/map_serializer.cpp
    src/config_loader.cpp
)

set(HEADERS
    src/map_editor.hpp
    src/map_serializer.hpp
    src/entity_data.hpp
    src/asset_info.hpp
    src/config_loader.hpp
    src/json.hpp
)

# Create executable
add_executable(rtype-map-editor ${SOURCES} ${HEADERS})

# Include directories (json.hpp is in src/)
target_include_directories(rtype-map-editor PRIVATE 
    ${RAYLIB_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link libraries
target_link_libraries(rtype-map-editor PRIVATE 
    ${RAYLIB_LIBRARY}
    ${GLFW_LIBRARY}
    GL
    m
    pthread
    dl
    rt
    X11
)

# Compiler warnings
if(MSVC)
    target_compile_options(rtype-map-editor PRIVATE /W4)
else()
    target_compile_options(rtype-map-editor PRIVATE -Wall -Wextra -pedantic)
endif()
