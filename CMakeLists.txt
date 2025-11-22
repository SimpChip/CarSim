cmake_minimum_required(VERSION 3.19)
project(CarSim)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

include(FetchContent)

# --- Threepp Setup ---
set(THREEPP_BUILD_TESTS OFF)
set(THREEPP_BUILD_EXAMPLES OFF)

FetchContent_Declare(
        threepp
        GIT_REPOSITORY https://github.com/markaren/threepp.git
        GIT_TAG ada259b72435e380c8e3def429fc8001233b1a8f
)

FetchContent_MakeAvailable(threepp)

# --- ImGui Setup ---
FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG master
)

FetchContent_MakeAvailable(imgui)

add_subdirectory(src)

# --- Resources ---
file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
target_compile_definitions(RacingGame PUBLIC DATA_DIR="${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources")
