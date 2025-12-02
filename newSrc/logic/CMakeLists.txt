add_library(logic
        ${PROJECT_SOURCE_DIR}/newSrc/logic/CarSystem.cpp
        ${PROJECT_SOURCE_DIR}/newSrc/logic/GameLogicSystem.cpp
        ${PROJECT_SOURCE_DIR}/newSrc/logic/LapSystem.cpp
        ${PROJECT_SOURCE_DIR}/newSrc/logic/MapSystem.cpp
        ${PROJECT_SOURCE_DIR}/newSrc/logic/PickupSystem.cpp
        ${PROJECT_SOURCE_DIR}/newSrc/logic/TrackGenerator.cpp
)

target_include_directories(logic PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(logic PUBLIC threepp::threepp collision entities controllers)