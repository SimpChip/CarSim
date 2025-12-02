add_library(ui
        ${PROJECT_SOURCE_DIR}/src/ui/MainMenuUI.cpp
        ${PROJECT_SOURCE_DIR}/src/ui/MapMakerUI.cpp
        ${PROJECT_SOURCE_DIR}/src/ui/RaceHUD.cpp
        ${PROJECT_SOURCE_DIR}/src/ui/UIManager.cpp
)
target_include_directories(ui PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(ui PUBLIC threepp::threepp imgui helpers controllers)