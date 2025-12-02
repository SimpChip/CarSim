add_library(ui
        ${PROJECT_SOURCE_DIR}/newSrc/ui/MainMenuUI.cpp
        ${PROJECT_SOURCE_DIR}/newSrc/ui/MapMakerUI.cpp
        ${PROJECT_SOURCE_DIR}/newSrc/ui/RaceHUD.cpp
        ${PROJECT_SOURCE_DIR}/newSrc/ui/UIManager.cpp
)
target_include_directories(ui PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(ui PUBLIC threepp::threepp imgui helpers controllers)