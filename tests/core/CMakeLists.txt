file(GLOB CORETEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(coretests ${CORETEST_SOURCES})

target_link_libraries(coretests
        PRIVATE
        Catch2::Catch2WithMain
        threepp::threepp
        core
)

include(Catch)
catch_discover_tests(coretests)

