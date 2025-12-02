file(GLOB SCENETEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(scenetests ${SCENETEST_SOURCES})

target_link_libraries(scenetests
        PRIVATE
        Catch2::Catch2WithMain
        threepp::threepp
        scenes visuals
)

include(Catch)
catch_discover_tests(scenetests)

