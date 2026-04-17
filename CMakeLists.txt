cmake_minimum_required(VERSION 3.15)
project(engine)

find_package(Vulkan REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor
    ${Vulkan_INCLUDE_DIRS}
)

file(GLOB_RECURSE SOURCES "src/*.c")
file(GLOB_RECURSE HEADERS "src/*.h")

add_executable(engine ${SOURCES} ${HEADERS})
target_link_libraries(engine ${Vulkan_LIBRARIES})  # automatisch auf vulkan-1.lib