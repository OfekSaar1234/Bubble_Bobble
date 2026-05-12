cmake_minimum_required(VERSION 3.31)
project(BubbleBobbleProject)

set(CMAKE_CXX_STANDARD 20)

# Disable Precompiled Headers (PCH) to fix the MinGW/GCC compilation errors
set(CMAKE_DISABLE_PRECOMPILE_HEADERS ON)

add_executable(${PROJECT_NAME}
        main.cpp
        bagel.h
        bubble_bobble.h
        bubble_bobble.cpp
        game.cpp
        game.h
)

# Use CACHE FORCE to ensure CMake doesn't ignore our requests
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/SDL)
target_link_libraries(${PROJECT_NAME} PUBLIC SDL3-static)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/SDL_image)
target_link_libraries(${PROJECT_NAME} PUBLIC SDL3_image-static)

# Included because the professor's new engine links it
set(BOX2D_SAMPLES OFF CACHE BOOL "" FORCE)
set(BOX2D_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(BOX2D_DOCS OFF CACHE BOOL "" FORCE)
set(BOX2D_PROFILE OFF CACHE BOOL "" FORCE)
set(BOX2D_VALIDATE OFF CACHE BOOL "" FORCE)
set(BOX2D_UNIT_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/box2d)
target_link_libraries(${PROJECT_NAME} PUBLIC box2d)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        "${PROJECT_SOURCE_DIR}/res"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/res"
)