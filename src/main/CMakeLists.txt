add_executable(RacingGame main.cpp)

target_link_libraries(RacingGame
        PRIVATE
        core
        logic
        render
        threepp::threepp
        glfw
        opengl32
)


# Add ImGui sources directly to the executable
target_sources(RacingGame PRIVATE
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# Include paths
target_include_directories(RacingGame PRIVATE
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
        ${CMAKE_SOURCE_DIR}/external
)