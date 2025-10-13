cmake_minimum_required(VERSION 3.19)
project(Oving4)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")

include(FetchContent)

set(THREEPP_BUILD_TESTS OFF)
set(THREEPP_BUILD_EXAMPLES OFF)
FetchContent_Declare(
        threepp
        GIT_REPOSITORY https://github.com/markaren/threepp.git
        GIT_TAG ada259b72435e380c8e3def429fc8001233b1a8f
)
FetchContent_MakeAvailable(threepp)

add_executable(threepp_demo main.cpp)
target_link_libraries(threepp_demo PRIVATE threepp::threepp)