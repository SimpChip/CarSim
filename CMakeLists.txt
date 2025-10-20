cmake_minimum_required(VERSION 3.19)
project(CarSim)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

include(FetchContent)

set(THREEPP_BUILD_TESTS OFF)
set(THREEPP_BUILD_EXAMPLES OFF)
FetchContent_Declare(
        threepp
        GIT_REPOSITORY https://github.com/markaren/threepp.git
        GIT_TAG ada259b72435e380c8e3def429fc8001233b1a8f
)
FetchContent_MakeAvailable(threepp)

add_executable(CarSimulator carsim.cpp carLogic.cpp
        RaceGame.cpp
        GameScene.cpp)
target_link_libraries(CarSimulator PRIVATE threepp::threepp)
target_include_directories(CarSimulator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(CarSimulator PRIVATE external)
