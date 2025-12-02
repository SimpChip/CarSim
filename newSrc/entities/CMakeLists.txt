add_library(entities
        ${PROJECT_SOURCE_DIR}/newSrc/entities/Car.cpp
        ${PROJECT_SOURCE_DIR}/newSrc/entities/Pickup.cpp
        ${PROJECT_SOURCE_DIR}/newSrc/entities/Checkpoint.cpp
)

target_include_directories(entities PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(entities PUBLIC threepp::threepp collision)