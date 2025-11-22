add_library(render INTERFACE)
target_include_directories(render INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(render INTERFACE core threepp::threepp)

