file(GLOB CONTROLLERS_SRC *.cpp)
add_library(controllers ${CONTROLLERS_SRC})
target_include_directories(controllers PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(controllers PUBLIC threepp::threepp logic)