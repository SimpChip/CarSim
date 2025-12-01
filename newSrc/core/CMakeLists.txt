file(GLOB CORE_SRC *.cpp)
add_library(core ${CORE_SRC})
target_include_directories(core PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(core PUBLIC imgui threepp::threepp logic ui scenes controllers nlohmann_json::nlohmann_json)