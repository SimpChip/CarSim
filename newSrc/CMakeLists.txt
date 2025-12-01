
add_subdirectory(logic)
add_subdirectory(collision)
add_subdirectory(controllers)
add_subdirectory(core)
add_subdirectory(scenes)
add_subdirectory(visuals)
add_subdirectory(ui)

add_executable(CarSim main.cpp)
target_link_libraries(CarSim
        PRIVATE core logic ui
        threepp::threepp imgui nlohmann_json::nlohmann_json
)
