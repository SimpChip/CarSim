add_library(collision
        ${PROJECT_SOURCE_DIR}/newSrc/collision/CollisionManager.cpp
        ${PROJECT_SOURCE_DIR}/newSrc/collision/OBB2D.cpp
)
target_include_directories(collision PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(collision PUBLIC threepp::threepp)