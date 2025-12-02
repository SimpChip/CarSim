add_library(visuals
        ${PROJECT_SOURCE_DIR}/src/visuals/CarVisuals.cpp
)

target_include_directories(visuals PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(visuals PUBLIC threepp::threepp)