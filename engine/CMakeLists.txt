cmake_minimum_required(VERSION 3.15)
project(engine)

find_package(Vulkan REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Vulkan_INCLUDE_DIRS}
)

file(GLOB_RECURSE SOURCES "src/*.c")
file(GLOB_RECURSE HEADERS "src/*.h")

add_compile_definitions(_DEBUG)

add_library(engine SHARED ${SOURCES} ${HEADERS}
    src/renderer/camera.h
    src/containers/hashtable.h
    src/systems/texture_system.h
    src/systems/texture_system.c
    src/memory/linear_allocator.h
    src/memory/linear_allocator.c
    src/systems/material_system.h
    src/systems/material_system.c
    src/systems/geometry_system.c)
target_link_libraries(engine ${Vulkan_LIBRARIES})  # automatisch auf vulkan-1.lib