file(GLOB COLLISION_SRC *.cpp)
add_library(collision ${COLLISION_SRC})
target_include_directories(collision PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(collision PUBLIC threepp::threepp)