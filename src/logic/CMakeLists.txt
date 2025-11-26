add_library(logic INTERFACE)

target_include_directories(logic INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(logic INTERFACE core threepp::threepp)