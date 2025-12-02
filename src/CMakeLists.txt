
add_subdirectory(core)
add_subdirectory(logic)
add_subdirectory(collision)
add_subdirectory(controllers)
add_subdirectory(scenes)
add_subdirectory(visuals)
add_subdirectory(ui)
add_subdirectory(entities)
add_subdirectory(helpers)

add_executable(CarSim main.cpp)
target_link_libraries(CarSim
        PRIVATE core
        threepp::threepp imgui nlohmann_json::nlohmann_json
)
