file(GLOB SCENES_SRC *.cpp)
add_library(scenes ${SCENES_SRC})
target_include_directories(scenes PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(scenes PUBLIC threepp::threepp visuals)