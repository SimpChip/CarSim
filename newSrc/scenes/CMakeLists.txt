
add_library(scenes
        ${PROJECT_SOURCE_DIR}/newSrc/scenes/GameScene.cpp)
target_include_directories(scenes PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(scenes PUBLIC threepp::threepp visuals helpers)