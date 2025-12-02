add_library(logic
        ${PROJECT_SOURCE_DIR}/src/logic/CarSystem.cpp
        ${PROJECT_SOURCE_DIR}/src/logic/GameLogicSystem.cpp
        ${PROJECT_SOURCE_DIR}/src/logic/LapSystem.cpp
        ${PROJECT_SOURCE_DIR}/src/logic/MapSystem.cpp
        ${PROJECT_SOURCE_DIR}/src/logic/PickupSystem.cpp
        ${PROJECT_SOURCE_DIR}/src/logic/TrackGenerator.cpp
)

target_include_directories(logic PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(logic PUBLIC threepp::threepp collision entities controllers)