file(GLOB UI_SRC *.cpp)
add_library(ui ${UI_SRC})
target_include_directories(ui PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(ui PUBLIC threepp::threepp)