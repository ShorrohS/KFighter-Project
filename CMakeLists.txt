cmake_minimum_required(VERSION 3.20)
project(kfighter VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Enable unit testing
enable_testing()

# Core engine static library
add_subdirectory(engine)

# Automated unit tests
add_subdirectory(tests)

# Fetch SDL2 for Desktop Interactive Test Harness
message(STATUS "Fetching SDL2 (release-2.30.2)...")
FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-2.30.2
    GIT_SHALLOW    TRUE
)

set(SDL_SHARED ON CACHE BOOL "Build SDL shared library" FORCE)
set(SDL_STATIC OFF CACHE BOOL "Build SDL static library" FORCE)
set(SDL_TEST OFF CACHE BOOL "Build SDL test targets" FORCE)

FetchContent_MakeAvailable(SDL2)

# Desktop interactive test harness
add_subdirectory(desktop)
