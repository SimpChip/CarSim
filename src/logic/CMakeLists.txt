add_library(logic INTERFACE
        data/types.h)
target_include_directories(logic INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(logic INTERFACE core threepp::threepp)