add_library(helpers
        ${PROJECT_SOURCE_DIR}/src/helpers/FileManager.cpp
        ${PROJECT_SOURCE_DIR}/src/helpers/ResourceManager.cpp
)
target_include_directories(helpers PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(helpers PUBLIC threepp::threepp nlohmann_json::nlohmann_json)