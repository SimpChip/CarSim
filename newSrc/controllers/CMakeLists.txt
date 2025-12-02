add_library(controllers
        ${PROJECT_SOURCE_DIR}/newSrc/controllers/PlayerController.cpp
        ${PROJECT_SOURCE_DIR}/newSrc/controllers/AiController.cpp
)
target_include_directories(controllers PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(controllers PUBLIC threepp::threepp entities)