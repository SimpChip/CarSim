file(GLOB COLITEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(colitests ${COLITEST_SOURCES})

target_link_libraries(colitests
        PRIVATE
        Catch2::Catch2WithMain
        threepp::threepp
        collision
)

include(Catch)
catch_discover_tests(colitests)

