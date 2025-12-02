add_library(core
        ${PROJECT_SOURCE_DIR}/newSrc/core/GameManager.cpp
)
target_include_directories(core PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(core PUBLIC imgui threepp::threepp logic ui scenes controllers helpers)