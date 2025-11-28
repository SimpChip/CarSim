cmake_minimum_required(VERSION 3.19)
project(CarSim)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_compile_definitions(DATA_DIR="${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources")
include(FetchContent)
# --- Threepp Setup ---
set(THREEPP_BUILD_TESTS OFF)
set(THREEPP_BUILD_EXAMPLES OFF)
FetchContent_Declare( threepp GIT_REPOSITORY https://github.com/markaren/threepp.git GIT_TAG ada259b72435e380c8e3def429fc8001233b1a8f )
FetchContent_MakeAvailable(threepp)
# --- ImGui Setup ---
FetchContent_Declare( imgui GIT_REPOSITORY https://github.com/ocornut/imgui.git GIT_TAG master )
FetchContent_MakeAvailable(imgui)
add_library(imgui STATIC ${imgui_SOURCE_DIR}/imgui.cpp ${imgui_SOURCE_DIR}/imgui_draw.cpp ${imgui_SOURCE_DIR}/imgui_tables.cpp ${imgui_SOURCE_DIR}/imgui_widgets.cpp ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp )
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends )
find_package(OpenGL REQUIRED)
target_link_libraries(imgui PUBLIC glfw OpenGL::GL)
include_directories(${PROJECT_SOURCE_DIR}/include)

file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_subdirectory(newSrc)