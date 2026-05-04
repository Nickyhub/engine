cmake_minimum_required(VERSION 3.15)
project(game)

file(GLOB_RECURSE SOURCES "src/*.c")
file(GLOB_RECURSE HEADERS "src/*.h")

add_executable(game ${SOURCES} ${HEADERS})

# engine includes verfügbar machen
target_include_directories(game PRIVATE
    ${CMAKE_SOURCE_DIR}/engine/src
)

# gegen engine.dll linken
target_link_libraries(game PRIVATE engine)