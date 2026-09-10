cmake_minimum_required(VERSION 3.16)

project(SS004Tetris VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(tetris_core
    src/core/ConsoleRenderer.cpp
    src/core/Game.cpp
    src/core/GameBoard.cpp
    src/core/Input.cpp
)

# Feature implementations are discovered automatically so each member can work
# mainly inside their own feature folder without editing this shared file.
file(GLOB_RECURSE TETRIS_FEATURE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/features/*.cpp"
)

if(TETRIS_FEATURE_SOURCES)
    target_sources(tetris_core PRIVATE ${TETRIS_FEATURE_SOURCES})
endif()

target_include_directories(tetris_core PUBLIC include)

if(MSVC)
    target_compile_options(tetris_core PRIVATE /W4)
else()
    target_compile_options(tetris_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(tetris src/main.cpp)
target_link_libraries(tetris PRIVATE tetris_core)

enable_testing()

add_executable(core_test tests/core_test.cpp)
target_link_libraries(core_test PRIVATE tetris_core)

add_executable(tetromino_test tests/tetromino_test.cpp)
target_link_libraries(tetromino_test PRIVATE tetris_core)

add_test(NAME core_test COMMAND core_test)
add_test(NAME tetromino_test COMMAND tetromino_test)
