file(GLOB LOGIC_SRC *.cpp)
add_library(logic ${LOGIC_SRC})
target_include_directories(logic PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(logic PUBLIC threepp::threepp collision)